/******************************************************************************
 *                                                                            *
 * Copyright (c) 2024, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         simple_archive.c
 *
 * This file implements a very small SquashFS writer that is only capable of
 * creating archives that contain a handful of files in the root directory.
 * The implementation is deliberately tiny and only implements what is needed
 * for the unit tests in this repository.  It should not be used as a full
 * featured replacement for mksquashfs.
 */

#define _DEFAULT_SOURCE

#include <mksqsh_archive_private.h>
#include <mksqsh_file_private.h>
#include <mksqsh_metablock.h>

#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <cextras/memory.h>
#include <endian.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int
mksqsh_archive_init(struct MksqshArchive *archive) {
	cx_prealloc_pool_init(&archive->file_pool, sizeof(struct MksqshFile));
	archive->root = mksqsh_file_new(archive, MKSQSH_FILE_TYPE_DIR, NULL);
	if (archive->root == NULL) {
		cx_prealloc_pool_cleanup(&archive->file_pool);
		return -SQSH_ERROR_MALLOC_FAILED;
	}
	return 0;
}

struct MksqshFile *
mksqsh_archive_root(struct MksqshArchive *archive) {
	if (archive->root == NULL) {
		return NULL;
	}
	return mksqsh_file_retain(archive->root);
}

int
mksqsh_archive_cleanup(struct MksqshArchive *archive) {
	if (archive->root != NULL) {
		mksqsh_file_release(archive->root);
		archive->root = NULL;
	}
	cx_prealloc_pool_cleanup(&archive->file_pool);
	return 0;
}

struct MksqshFileSpec {
	struct MksqshFile *file;
	enum MksqshFileContentType type;
	const uint8_t *data;
	FILE *fd;
	size_t size;
	bool close_fd;
};

/*  Helper structure to keep track of generated inode references. */
struct FileInfo {
	const struct MksqshFileSpec *spec;
	uint64_t inode_ref;
};

static int
write_inode_table(
		struct MksqshMetablock *mb, struct FileInfo *files, size_t file_count,
		uint32_t block_size, uint64_t data_offset, uint64_t dir_size,
		uint64_t *root_ref) {
	/* Compute sizes of inodes to know offsets. */
	size_t root_inode_size = sizeof(struct SqshDataInodeHeader) +
			sizeof(struct SqshDataInodeDirectoryExt);

	size_t *file_inode_size = calloc(file_count, sizeof(size_t));
	if (file_count > 0 && file_inode_size == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}

	size_t offset = root_inode_size;
	for (size_t i = 0; i < file_count; i++) {
		const size_t blocks =
				(files[i].spec->size + block_size - 1) / block_size;
		file_inode_size[i] = sizeof(struct SqshDataInodeHeader) +
				sizeof(struct SqshDataInodeFile) + blocks * sizeof(uint32_t);
		files[i].inode_ref = sqsh_address_ref_create(0, (uint16_t)offset);
		offset += file_inode_size[i];
	}

	struct SqshDataInode root = {0};
	root.header.type = htole16(SQSH_INODE_TYPE_EXTENDED_DIRECTORY);
	root.header.permissions = htole16(0755);
	root.header.uid_idx = htole16(0);
	root.header.gid_idx = htole16(0);
	root.header.modified_time = htole32(0);
	root.header.inode_number = htole32(1);

	root.data.directory_ext.hard_link_count = htole32(2);
	root.data.directory_ext.file_size = htole32(dir_size + 3);
	root.data.directory_ext.block_start = htole32(0);
	root.data.directory_ext.parent_inode_number = htole32(1);
	root.data.directory_ext.index_count = htole16(0);
	root.data.directory_ext.block_offset = htole16(0);
	root.data.directory_ext.xattr_idx = htole32(0);

	*root_ref = mksqsh__metablock_ref(mb);
	int rv = mksqsh__metablock_write(mb, (uint8_t *)&root, root_inode_size);
	if (rv < 0) {
		free(file_inode_size);
		return rv;
	}

	/* Now file inodes */
	uint64_t current_data = data_offset;
	for (size_t i = 0; i < file_count; i++) {
		const struct MksqshFileSpec *spec = files[i].spec;
		struct SqshDataInode inode = {0};
		inode.header.type = htole16(SQSH_INODE_TYPE_BASIC_FILE);
		inode.header.permissions = htole16(0644);
		inode.header.uid_idx = htole16(0);
		inode.header.gid_idx = htole16(0);
		inode.header.modified_time = htole32(0);
		inode.header.inode_number = htole32((uint32_t)(i + 2));

		inode.data.file.blocks_start = htole32((uint32_t)current_data);
		inode.data.file.fragment_block_index = htole32(0xFFFFFFFFu);
		inode.data.file.block_offset = htole32(0);
		inode.data.file.file_size = htole32((uint32_t)spec->size);

		uint32_t size_info =
				(uint32_t)spec->size | (1u << 24); /* uncompressed */

		rv = mksqsh__metablock_write(
				mb, (uint8_t *)&inode,
				sizeof(struct SqshDataInodeHeader) +
						sizeof(struct SqshDataInodeFile));
		if (rv < 0) {
			free(file_inode_size);
			return rv;
		}
		rv = mksqsh__metablock_write(
				mb, (uint8_t *)&size_info, sizeof(uint32_t));
		if (rv < 0) {
			free(file_inode_size);
			return rv;
		}

		current_data += spec->size;
	}

	free(file_inode_size);
	return 0;
}

static int
write_directory_table(
		struct MksqshMetablock *mb, struct MksqshNode **nodes,
		const size_t *node_to_file, struct FileInfo *files, size_t node_count) {
	struct SqshDataDirectoryFragment fragment = {0};
	fragment.count = htole32((uint32_t)(node_count == 0 ? 0 : node_count - 1));
	fragment.start = htole32(0);
	fragment.inode_number = htole32(1);

	int rv = mksqsh__metablock_write(
			mb, (uint8_t *)&fragment, sizeof(struct SqshDataDirectoryFragment));
	if (rv < 0) {
		return rv;
	}

	for (size_t i = 0; i < node_count; i++) {
		struct SqshDataDirectoryEntry entry = {0};
		size_t file_index = node_to_file[i];

		entry.offset = htole16((uint16_t)sqsh_address_ref_inner_offset(
				files[file_index].inode_ref));
		entry.inode_offset = htole16((uint16_t)(file_index + 1));
		entry.type = htole16(SQSH_INODE_TYPE_BASIC_FILE);
		entry.name_size = htole16((uint16_t)(strlen(nodes[i]->name) - 1));

		rv = mksqsh__metablock_write(
				mb, (uint8_t *)&entry, sizeof(struct SqshDataDirectoryEntry));
		if (rv < 0) {
			return rv;
		}
		rv = mksqsh__metablock_write(
				mb, (const uint8_t *)nodes[i]->name, strlen(nodes[i]->name));
		if (rv < 0) {
			return rv;
		}
	}

	return 0;
}

static int
mksqsh__create_archive(
		FILE *out, struct MksqshNode **nodes, size_t node_count) {
	if (out == NULL) {
		return -SQSH_ERROR_INTERNAL;
	}

	int rv = 0;
	const uint32_t block_size = 4096;
	struct MksqshSuperblock superblock = {0};

	struct MksqshFile **unique_files =
			calloc(node_count, sizeof(*unique_files));
	size_t *node_to_file = calloc(node_count, sizeof(size_t));
	struct MksqshFileSpec *files = calloc(node_count, sizeof(*files));
	struct FileInfo *info = NULL;
	size_t file_count = 0;

	if ((node_count > 0 &&
		 (unique_files == NULL || node_to_file == NULL || files == NULL))) {
		free(unique_files);
		free(node_to_file);
		free(files);
		return -SQSH_ERROR_MALLOC_FAILED;
	}

	for (size_t i = 0; i < node_count; i++) {
		struct MksqshFile *file = nodes[i]->file;
		size_t j = 0;
		for (; j < file_count; j++) {
			if (unique_files[j] == file) {
				break;
			}
		}
		if (j == file_count) {
			unique_files[file_count] = file;
			files[file_count].file = file;
			switch (file->content_type) {
			case MKSQSH_FILE_CONTENT_MEMORY:
				files[file_count].type = MKSQSH_FILE_CONTENT_MEMORY;
				files[file_count].data = file->data;
				files[file_count].size = file->size;
				break;
			case MKSQSH_FILE_CONTENT_FD:
				files[file_count].type = MKSQSH_FILE_CONTENT_FD;
				files[file_count].fd = file->fd;
				if (fseek(file->fd, 0, SEEK_END) < 0) {
					rv = -SQSH_ERROR_INTERNAL;
					goto out;
				}
				files[file_count].size = (size_t)ftell(file->fd);
				rewind(file->fd);
				break;
			case MKSQSH_FILE_CONTENT_PATH: {
				FILE *fd = fopen(file->path, "rb");
				if (fd == NULL) {
					rv = -SQSH_ERROR_INTERNAL;
					goto out;
				}
				if (fseek(fd, 0, SEEK_END) < 0) {
					fclose(fd);
					rv = -SQSH_ERROR_INTERNAL;
					goto out;
				}
				files[file_count].fd = fd;
				files[file_count].size = (size_t)ftell(fd);
				rewind(fd);
				files[file_count].type = MKSQSH_FILE_CONTENT_PATH;
				files[file_count].close_fd = true;
				break;
			}
			default:
				break;
			}
			file_count++;
		}
		node_to_file[i] = j;
	}

	info = calloc(file_count, sizeof(*info));
	if (file_count > 0 && info == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	for (size_t i = 0; i < file_count; i++) {
		info[i].spec = &files[i];
	}

	mksqsh__superblock_init(&superblock);
	mksqsh__superblock_compression_id(&superblock, SQSH_COMPRESSION_GZIP);
	mksqsh__superblock_compress_inodes(&superblock, false);
	mksqsh__superblock_compress_data(&superblock, false);
	mksqsh__superblock_compress_fragments(&superblock, false);
	mksqsh__superblock_fragment_table_start(&superblock, UINT64_MAX);
	mksqsh__superblock_export_table_start(&superblock, 0);
	mksqsh__superblock_xattr_table_start(&superblock, UINT64_MAX);
	mksqsh__superblock_fragment_count(&superblock, 0);
	mksqsh__superblock_id_count(&superblock, 1);

	uint8_t zero[sizeof(struct SqshDataSuperblock)] = {0};
	if (fwrite(zero, sizeof(zero), 1, out) != 1) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	uint64_t data_offset = sizeof(struct SqshDataSuperblock);
	for (size_t i = 0; i < file_count; i++) {
		if (files[i].size > 0) {
			switch (files[i].type) {
			case MKSQSH_FILE_CONTENT_MEMORY:
				if (fwrite(files[i].data, files[i].size, 1, out) != 1) {
					rv = -SQSH_ERROR_INTERNAL;
					goto out;
				}
				break;
			case MKSQSH_FILE_CONTENT_FD:
			case MKSQSH_FILE_CONTENT_PATH: {
				size_t remaining = files[i].size;
				uint8_t buf[4096];
				while (remaining > 0) {
					size_t chunk =
							remaining > sizeof(buf) ? sizeof(buf) : remaining;
					if (fread(buf, chunk, 1, files[i].fd) != 1) {
						rv = -SQSH_ERROR_INTERNAL;
						goto out;
					}
					if (fwrite(buf, chunk, 1, out) != 1) {
						rv = -SQSH_ERROR_INTERNAL;
						goto out;
					}
					remaining -= chunk;
				}
				if (files[i].close_fd && files[i].fd != NULL) {
					fclose(files[i].fd);
				}
			} break;
			default:
				break;
			}
		}
		data_offset += files[i].size;
	}

	uint64_t inode_table_start = ftell(out);

	size_t dir_entries_size = 0;
	for (size_t i = 0; i < node_count; i++) {
		dir_entries_size +=
				sizeof(struct SqshDataDirectoryEntry) + strlen(nodes[i]->name);
	}
	size_t dir_size =
			sizeof(struct SqshDataDirectoryFragment) + dir_entries_size;

	struct MksqshMetablock inode_mb = {0};
	mksqsh__metablock_init(&inode_mb, out);
	uint64_t root_ref;
	rv = write_inode_table(
			&inode_mb, info, file_count, block_size,
			sizeof(struct SqshDataSuperblock), dir_size, &root_ref);
	if (rv < 0) {
		goto out;
	}
	rv = mksqsh__metablock_flush(&inode_mb);
	if (rv < 0) {
		goto out;
	}

	uint64_t directory_table_start = ftell(out);

	struct MksqshMetablock dir_mb = {0};
	mksqsh__metablock_init(&dir_mb, out);
	rv = write_directory_table(&dir_mb, nodes, node_to_file, info, node_count);
	if (rv < 0) {
		goto out;
	}
	rv = mksqsh__metablock_flush(&dir_mb);
	if (rv < 0) {
		goto out;
	}

	uint64_t id_table_lookup_start = ftell(out);
	uint64_t aligned = (id_table_lookup_start + 7) & ~((uint64_t)7);
	while (id_table_lookup_start < aligned) {
		fputc('\0', out);
		id_table_lookup_start++;
	}
	const uint32_t id = 0;
	if (fwrite(&id, sizeof(id), 1, out) != 1) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	uint64_t superblock_start = 0;
	uint64_t superblock_end = ftell(out);
	uint64_t total_size = superblock_end - superblock_start;
	mksqsh__superblock_bytes_used(&superblock, total_size);
	mksqsh__superblock_inode_count(&superblock, (uint32_t)(file_count + 1));
	mksqsh__superblock_inode_table_start(&superblock, inode_table_start);
	mksqsh__superblock_directory_table_start(
			&superblock, directory_table_start);
	mksqsh__superblock_id_table_start(&superblock, id_table_lookup_start);

	fseek(out, 0, SEEK_SET);
	rv = mksqsh__superblock_write(&superblock, out);

out:
	free(unique_files);
	free(node_to_file);
	free(files);
	free(info);
	return rv < 0 ? rv : 0;
}

int
mksqsh_archive_write(struct MksqshArchive *archive, const char *path) {
	if (archive == NULL || archive->root == NULL || path == NULL) {
		return -SQSH_ERROR_INTERNAL;
	}

	struct MksqshNode *child = archive->root->children;
	size_t node_count = 0;
	for (struct MksqshNode *n = child; n != NULL; n = n->next) {
		if (n->file->type != MKSQSH_FILE_TYPE_REG) {
			return -SQSH_ERROR_INTERNAL;
		}
		node_count++;
	}

	struct MksqshNode **nodes = calloc(node_count, sizeof(*nodes));
	if (node_count > 0 && nodes == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
	size_t i = 0;
	for (struct MksqshNode *n = child; n != NULL; n = n->next, i++) {
		nodes[i] = n;
	}

	FILE *out = fopen(path, "wb+");
	if (out == NULL) {
		free(nodes);
		return -SQSH_ERROR_INTERNAL;
	}
	int rv = mksqsh__create_archive(out, nodes, node_count);
	fclose(out);
	free(nodes);
	return rv;
}

struct CxPreallocPool *
mksqsh__archive_file_pool(struct MksqshArchive *archive) {
	return &archive->file_pool;
}
