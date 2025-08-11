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

#include <mksqsh_file_private.h>
#include <mksqsh_archive_private.h>
#include <mksqsh_metablock.h>
#include <mksqsh_file_private.h>

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
        mksqsh_file_release(archive->root);
        archive->root = NULL;
        cx_prealloc_pool_cleanup(&archive->file_pool);
        return 0;
}

struct MksqshFileSpec {
        const char *name;
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
                struct MksqshMetablock *mb, struct FileInfo *files,
                size_t file_count, uint32_t block_size, uint64_t data_offset,
                uint64_t *root_ref) {
        /* Compute sizes of inodes to know offsets. */
        size_t root_inode_size =
                        sizeof(struct SqshDataInodeHeader) +
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
                                     sizeof(struct SqshDataInodeFile) +
                                     blocks * sizeof(uint32_t);
                files[i].inode_ref =
                                sqsh_address_ref_create(0, (uint16_t)offset);
                offset += file_inode_size[i];
        }

        /* Build root inode now that directory size is known later. */
        size_t dir_entries_size = 0;
        for (size_t i = 0; i < file_count; i++) {
                dir_entries_size += sizeof(struct SqshDataDirectoryEntry) +
                                    strlen(files[i].spec->name);
        }
        size_t dir_size = sizeof(struct SqshDataDirectoryFragment) + dir_entries_size;

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
                struct MksqshMetablock *mb, struct FileInfo *files,
                size_t file_count) {
        struct SqshDataDirectoryFragment fragment = {0};
        fragment.count = htole32((uint32_t)(file_count == 0 ? 0 : file_count - 1));
        fragment.start = htole32(0);
        fragment.inode_number = htole32(1);

        int rv = mksqsh__metablock_write(
                        mb, (uint8_t *)&fragment,
                        sizeof(struct SqshDataDirectoryFragment));
        if (rv < 0) {
                return rv;
        }

        for (size_t i = 0; i < file_count; i++) {
                const struct MksqshFileSpec *spec = files[i].spec;
                struct SqshDataDirectoryEntry entry = {0};

                entry.offset = htole16((uint16_t)sqsh_address_ref_inner_offset(
                                files[i].inode_ref));
                entry.inode_offset = htole16((uint16_t)(i + 1));
                entry.type = htole16(SQSH_INODE_TYPE_BASIC_FILE);
                entry.name_size = htole16((uint16_t)(strlen(spec->name) - 1));

                rv = mksqsh__metablock_write(
                                mb, (uint8_t *)&entry,
                                sizeof(struct SqshDataDirectoryEntry));
                if (rv < 0) {
                        return rv;
                }
                rv = mksqsh__metablock_write(
                                mb, (const uint8_t *)spec->name,
                                strlen(spec->name));
                if (rv < 0) {
                        return rv;
                }
        }

        return 0;
}

static int
mksqsh__create_archive(
                FILE *out, const struct MksqshFileSpec *files, size_t file_count) {
        if (out == NULL) {
                return -SQSH_ERROR_INTERNAL;
        }

        int rv = 0;
        const uint32_t block_size = 4096;
        struct MksqshSuperblock superblock = {0};
        struct FileInfo *info = NULL;

        info = calloc(file_count, sizeof(*info));
        if (file_count > 0 && info == NULL) {
                return -SQSH_ERROR_MALLOC_FAILED;
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

        /*  Reserve space for superblock */
        uint8_t zero[sizeof(struct SqshDataSuperblock)] = {0};
        if (fwrite(zero, sizeof(zero), 1, out) != 1) {
                free(info);
                return -SQSH_ERROR_INTERNAL;
        }

        /* Write data blocks */
        uint64_t data_offset = sizeof(struct SqshDataSuperblock);
        for (size_t i = 0; i < file_count; i++) {
                if (files[i].size > 0) {
                        switch (files[i].type) {
                        case MKSQSH_FILE_CONTENT_MEMORY:
                                if (fwrite(files[i].data, files[i].size, 1, out) != 1) {
                                        free(info);
                                        return -SQSH_ERROR_INTERNAL;
                                }
                                break;
                        case MKSQSH_FILE_CONTENT_FD:
                        case MKSQSH_FILE_CONTENT_PATH:
                                {
                                        size_t remaining = files[i].size;
                                        uint8_t buf[4096];
                                        while (remaining > 0) {
                                                size_t chunk = remaining > sizeof(buf) ? sizeof(buf) : remaining;
                                                if (fread(buf, chunk, 1, files[i].fd) != 1) {
                                                        free(info);
                                                        return -SQSH_ERROR_INTERNAL;
                                                }
                                                if (fwrite(buf, chunk, 1, out) != 1) {
                                                        free(info);
                                                        return -SQSH_ERROR_INTERNAL;
                                                }
                                                remaining -= chunk;
                                        }
                                        if (files[i].close_fd && files[i].fd != NULL) {
                                                fclose(files[i].fd);
                                        }
                                }
                                break;
                        default:
                                break;
                        }
                }
                data_offset += files[i].size;
        }

        uint64_t inode_table_start = ftell(out);

        struct MksqshMetablock inode_mb = {0};
        mksqsh__metablock_init(&inode_mb, out);
        uint64_t root_ref;
        rv = write_inode_table(
                        &inode_mb, info, file_count, block_size,
                        sizeof(struct SqshDataSuperblock), &root_ref);
        if (rv < 0) {
                free(info);
                return rv;
        }
        rv = mksqsh__metablock_flush(&inode_mb);
        if (rv < 0) {
                free(info);
                return rv;
        }

        uint64_t directory_table_start = ftell(out);

        struct MksqshMetablock dir_mb = {0};
        mksqsh__metablock_init(&dir_mb, out);
        rv = write_directory_table(&dir_mb, info, file_count);
        if (rv < 0) {
                free(info);
                return rv;
        }
        rv = mksqsh__metablock_flush(&dir_mb);
        if (rv < 0) {
                free(info);
                return rv;
        }

        uint64_t id_table_start_data = ftell(out);

        struct MksqshMetablock id_mb = {0};
        mksqsh__metablock_init(&id_mb, out);
        uint32_t zero_id = 0;
        rv = mksqsh__metablock_write(&id_mb, (uint8_t *)&zero_id, sizeof(uint32_t));
        if (rv < 0) {
                free(info);
                return rv;
        }
        rv = mksqsh__metablock_flush(&id_mb);
        if (rv < 0) {
                free(info);
                return rv;
        }

        uint64_t id_table_lookup_start = ftell(out);
        /* Align lookup table to 8 bytes as required by the SquashFS format. */
        uint64_t aligned = (id_table_lookup_start + 7) & ~((uint64_t)7);
        while (id_table_lookup_start < aligned) {
                fputc('\0', out);
                id_table_lookup_start++;
        }

        uint64_t abs = id_table_start_data;
        if (fwrite(&abs, sizeof(uint64_t), 1, out) != 1) {
                free(info);
                return -SQSH_ERROR_INTERNAL;
        }

        uint64_t bytes_used = ftell(out);

        mksqsh__superblock_root_inode_ref(&superblock, root_ref);
        mksqsh__superblock_inode_count(&superblock, (uint32_t)(file_count + 1));
        mksqsh__superblock_bytes_used(&superblock, bytes_used);
        mksqsh__superblock_inode_table_start(&superblock, inode_table_start);
        mksqsh__superblock_directory_table_start(&superblock, directory_table_start);
        mksqsh__superblock_id_table_start(&superblock, id_table_lookup_start);

        fseek(out, 0, SEEK_SET);
        rv = mksqsh__superblock_write(&superblock, out);

        free(info);
        return rv < 0 ? rv : 0;
}

int
mksqsh_archive_write(struct MksqshArchive *archive, const char *path) {
        if (archive == NULL || archive->root == NULL || path == NULL) {
                return -SQSH_ERROR_INTERNAL;
        }

        size_t file_count = 0;
        for (size_t i = 0; i < archive->root->child_count; i++) {
                if (archive->root->children[i]->type != MKSQSH_FILE_TYPE_REG) {
                        return -SQSH_ERROR_INTERNAL;
                }
                file_count++;
        }

        int rv = 0;
        struct MksqshFileSpec *files = calloc(file_count, sizeof(*files));
        if (file_count > 0 && files == NULL) {
                return -SQSH_ERROR_MALLOC_FAILED;
        }
        for (size_t i = 0; i < file_count; i++) {
                struct MksqshFile *child = archive->root->children[i];
                files[i].name = child->name;
                switch (child->content_type) {
                case MKSQSH_FILE_CONTENT_MEMORY:
                        files[i].type = MKSQSH_FILE_CONTENT_MEMORY;
                        files[i].data = child->data;
                        files[i].size = child->size;
                        break;
                case MKSQSH_FILE_CONTENT_FD: {
                        if (fseek(child->fd, 0, SEEK_END) < 0) {
                                rv = -SQSH_ERROR_INTERNAL;
                                goto out;
                        }
                        long sz = ftell(child->fd);
                        if (sz < 0) {
                                rv = -SQSH_ERROR_INTERNAL;
                                goto out;
                        }
                        rewind(child->fd);
                        files[i].type = MKSQSH_FILE_CONTENT_FD;
                        files[i].fd = child->fd;
                        files[i].size = (size_t)sz;
                        break;
                }
                case MKSQSH_FILE_CONTENT_PATH: {
                        FILE *fd = fopen(child->path, "rb");
                        if (fd == NULL) {
                                rv = -SQSH_ERROR_INTERNAL;
                                goto out;
                        }
                        if (fseek(fd, 0, SEEK_END) < 0) {
                                fclose(fd);
                                rv = -SQSH_ERROR_INTERNAL;
                                goto out;
                        }
                        long sz = ftell(fd);
                        if (sz < 0) {
                                fclose(fd);
                                rv = -SQSH_ERROR_INTERNAL;
                                goto out;
                        }
                        rewind(fd);
                        files[i].type = MKSQSH_FILE_CONTENT_PATH;
                        files[i].fd = fd;
                        files[i].size = (size_t)sz;
                        files[i].close_fd = true;
                        break;
                }
                default:
                        rv = -SQSH_ERROR_INTERNAL;
                        goto out;
                }
        }

        FILE *out = fopen(path, "wb+");
        if (out == NULL) {
                rv = -SQSH_ERROR_INTERNAL;
                goto out;
        }
        rv = mksqsh__create_archive(out, files, file_count);
        fclose(out);

out:
        if (rv < 0) {
                for (size_t i = 0; i < file_count; i++) {
                        if (files[i].close_fd && files[i].fd != NULL) {
                                fclose(files[i].fd);
                        }
                }
        }
        free(files);
        return rv;
}

struct CxPreallocPool *mksqsh__archive_file_pool(struct MksqshArchive *archive) {
	return &archive->file_pool;
}

