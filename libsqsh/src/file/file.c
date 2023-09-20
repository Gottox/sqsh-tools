/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         inode_context.c
 */

#include <sqsh_file_private.h>

#include "../utils/utils.h"
#include <cextras/utils.h>
#include <sqsh_archive.h>
#include <sqsh_error.h>
#include <sqsh_table.h>

#include <sqsh_data_private.h>
#include <sqsh_tree_private.h>

static const struct SqshDataInode *
get_inode(const struct SqshFile *inode) {
	return (const struct SqshDataInode *)sqsh__metablock_reader_data(
			&inode->metablock);
}

static int
inode_load(struct SqshFile *context) {
	int rv = 0;
	size_t size = sizeof(struct SqshDataInodeHeader);

	const struct SqshDataInode *inode = get_inode(context);
	const enum SqshDataInodeType type = sqsh__data_inode_type(inode);
	switch (type) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		size += sizeof(struct SqshDataInodeDirectory);
		break;
	case SQSH_INODE_TYPE_BASIC_FILE:
		size += sizeof(struct SqshDataInodeFile);
		break;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		size += sizeof(struct SqshDataInodeSymlink);
		break;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		size += sizeof(struct SqshDataInodeDevice);
		break;
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		size += sizeof(struct SqshDataInodeIpc);
		break;
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		size += sizeof(struct SqshDataInodeDirectoryExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		size += sizeof(struct SqshDataInodeFileExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		size += sizeof(struct SqshDataInodeSymlinkExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		size += sizeof(struct SqshDataInodeDeviceExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		size += sizeof(struct SqshDataInodeIpcExt);
		break;
	default:
		return -SQSH_ERROR_UNKNOWN_FILE_TYPE;
	}
	rv = sqsh__metablock_reader_advance(&context->metablock, 0, size);
	if (rv < 0) {
		return rv;
	}

	/* The pointer may has been invalidated by reader_advance, so retrieve it
	 * again.
	 */
	inode = get_inode(context);
	switch (type) {
	case SQSH_INODE_TYPE_EXTENDED_FILE:
	case SQSH_INODE_TYPE_BASIC_FILE:
		size += sqsh_file_block_count(context) * sizeof(uint32_t);
		break;
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		size += sqsh__data_inode_directory_ext_index_count(
				sqsh__data_inode_directory_ext(inode));
		break;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		size += sqsh_file_symlink_size(context);
		break;
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		size += sqsh_file_symlink_size(context);
		/* xattr index */
		size += sizeof(uint32_t);
	default:
		/* nop */
		break;
	}
	rv = sqsh__metablock_reader_advance(&context->metablock, 0, size);

	return rv;
}

static uint32_t
get_size_info(const struct SqshFile *context, sqsh_index_t index) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh__data_inode_file(inode);
		return sqsh__data_inode_file_block_size_info(basic_file, index);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh__data_inode_file_ext(inode);
		return sqsh__data_inode_file_ext_block_size_info(extended_file, index);
	}
	/* Should never happen */
	abort();
}

int
sqsh__file_init(
		struct SqshFile *inode, struct SqshArchive *archive,
		uint64_t inode_ref) {
	const uint32_t outer_offset = sqsh_address_ref_outer_offset(inode_ref);
	const uint16_t inner_offset = sqsh_address_ref_inner_offset(inode_ref);
	uint64_t address_outer;
	struct SqshInodeMap *inode_map;

	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

	const uint64_t inode_table_start =
			sqsh_superblock_inode_table_start(superblock);

	if (SQSH_ADD_OVERFLOW(inode_table_start, outer_offset, &address_outer)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	const uint64_t upper_limit =
			sqsh_superblock_directory_table_start(superblock);
	rv = sqsh__metablock_reader_init(
			&inode->metablock, archive, address_outer, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__metablock_reader_advance(
			&inode->metablock, inner_offset,
			sizeof(struct SqshDataInodeHeader));
	if (rv < 0) {
		goto out;
	}

	inode->archive = archive;
	inode->inode_ref = inode_ref;

	rv = inode_load(inode);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_archive_inode_map(archive, &inode_map);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_inode_map_set2(inode_map, sqsh_file_inode(inode), inode_ref);

out:
	if (rv < 0) {
		sqsh__file_cleanup(inode);
	}
	return rv;
}

struct SqshFile *
sqsh_open_by_ref(struct SqshArchive *sqsh, uint64_t inode_ref, int *err) {
	int rv = 0;
	struct SqshFile *inode = calloc(1, sizeof(struct SqshFile));
	if (inode == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = sqsh__file_init(inode, sqsh, inode_ref);
	if (rv < 0) {
		free(inode);
		inode = NULL;
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	return inode;
}

bool
sqsh_file_is_extended(const struct SqshFile *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
	case SQSH_INODE_TYPE_BASIC_FILE:
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		return false;
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
	case SQSH_INODE_TYPE_EXTENDED_FILE:
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return true;
	}
	return false;
}

uint32_t
sqsh_file_hard_link_count(const struct SqshFile *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		return sqsh__data_inode_directory_hard_link_count(
				sqsh__data_inode_directory(inode));
	case SQSH_INODE_TYPE_BASIC_FILE:
		return 1;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		return sqsh__data_inode_symlink_hard_link_count(
				sqsh__data_inode_symlink(inode));
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		return sqsh__data_inode_device_hard_link_count(
				sqsh__data_inode_device(inode));
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		return sqsh__data_inode_ipc_hard_link_count(
				sqsh__data_inode_ipc(inode));

	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return sqsh__data_inode_directory_ext_hard_link_count(
				sqsh__data_inode_directory_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return sqsh__data_inode_file_ext_hard_link_count(
				sqsh__data_inode_file_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return sqsh__data_inode_symlink_ext_hard_link_count(
				sqsh__data_inode_symlink_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return sqsh__data_inode_device_ext_hard_link_count(
				sqsh__data_inode_device_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return sqsh__data_inode_ipc_ext_hard_link_count(
				sqsh__data_inode_ipc_ext(inode));
	}
	return -SQSH_ERROR_UNKNOWN_FILE_TYPE;
}

uint64_t
sqsh_file_size(const struct SqshFile *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;
	const struct SqshDataInodeDirectory *basic_dir;
	const struct SqshDataInodeDirectoryExt *extended_dir;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = sqsh__data_inode_directory(inode);
		return sqsh__data_inode_directory_file_size(basic_dir);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = sqsh__data_inode_directory_ext(inode);
		return sqsh__data_inode_directory_ext_file_size(extended_dir);
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh__data_inode_file(inode);
		return sqsh__data_inode_file_size(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh__data_inode_file_ext(inode);
		return sqsh__data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
sqsh_file_permission(const struct SqshFile *inode) {
	return sqsh__data_inode_permissions(get_inode(inode));
}

uint32_t
sqsh_file_inode(const struct SqshFile *inode) {
	return sqsh__data_inode_number(get_inode(inode));
}

uint32_t
sqsh_file_modified_time(const struct SqshFile *inode) {
	return sqsh__data_inode_modified_time(get_inode(inode));
}

uint64_t
sqsh_file_blocks_start(const struct SqshFile *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh__data_inode_file(inode);
		return sqsh__data_inode_file_blocks_start(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh__data_inode_file_ext(inode);
		return sqsh__data_inode_file_ext_blocks_start(extended_file);
	}
	return UINT64_MAX;
}

uint32_t
sqsh_file_block_count(const struct SqshFile *context) {
	const struct SqshSuperblock *superblock =
			sqsh_archive_superblock(context->archive);
	uint64_t file_size = sqsh_file_size(context);
	uint32_t block_size = sqsh_superblock_block_size(superblock);

	if (file_size == UINT64_MAX) {
		return UINT32_MAX;
	} else if (file_size == 0) {
		return 0;
	} else if (sqsh_file_has_fragment(context)) {
		return file_size / block_size;
	} else {
		return SQSH_DIVIDE_CEIL(file_size, block_size);
	}
}

uint32_t
sqsh_file_block_size(const struct SqshFile *inode, uint32_t index) {
	const uint32_t size_info = get_size_info(inode, index);

	return sqsh_datablock_size(size_info);
}

bool
sqsh_file_block_is_compressed(const struct SqshFile *inode, uint32_t index) {
	const uint32_t size_info = get_size_info(inode, index);

	return sqsh_datablock_is_compressed(size_info);
}

uint32_t
sqsh_file_fragment_block_index(const struct SqshFile *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh__data_inode_file(inode);
		return sqsh__data_inode_file_fragment_block_index(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh__data_inode_file_ext(inode);
		return sqsh__data_inode_file_ext_fragment_block_index(extended_file);
	}
	return SQSH_INODE_NO_FRAGMENT;
}

uint32_t
sqsh_file_directory_block_start(const struct SqshFile *context) {
	const struct SqshDataInodeDirectory *basic_file;
	const struct SqshDataInodeDirectoryExt *extended_file;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_file = sqsh__data_inode_directory(inode);
		return sqsh__data_inode_directory_block_start(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_file = sqsh__data_inode_directory_ext(inode);
		return sqsh__data_inode_directory_ext_block_start(extended_file);
	}
	return UINT32_MAX;
}

uint32_t
sqsh_file_directory_block_offset(const struct SqshFile *context) {
	const struct SqshDataInodeDirectory *basic_directory;
	const struct SqshDataInodeDirectoryExt *extended_directory;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_directory = sqsh__data_inode_directory(inode);
		return sqsh__data_inode_directory_block_offset(basic_directory);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_directory = sqsh__data_inode_directory_ext(inode);
		return sqsh__data_inode_directory_ext_block_offset(extended_directory);
	}
	return UINT32_MAX;
}

uint32_t
sqsh_file_directory_parent_inode(const struct SqshFile *context) {
	const struct SqshDataInodeDirectory *basic_directory;
	const struct SqshDataInodeDirectoryExt *extended_directory;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_directory = sqsh__data_inode_directory(inode);
		return sqsh__data_inode_directory_parent_inode_number(basic_directory);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_directory = sqsh__data_inode_directory_ext(inode);
		return sqsh__data_inode_directory_ext_parent_inode_number(
				extended_directory);
	}
	return UINT32_MAX;
}

uint32_t
sqsh_file_fragment_block_offset(const struct SqshFile *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh__data_inode_file(inode);
		return sqsh__data_inode_file_block_offset(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh__data_inode_file_ext(inode);
		return sqsh__data_inode_file_ext_block_offset(extended_file);
	}
	return SQSH_INODE_NO_FRAGMENT;
}

bool
sqsh_file_has_fragment(const struct SqshFile *inode) {
	return sqsh_file_fragment_block_index(inode) != SQSH_INODE_NO_FRAGMENT;
}

enum SqshFileType
sqsh_file_type(const struct SqshFile *context) {
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return SQSH_FILE_TYPE_DIRECTORY;
	case SQSH_INODE_TYPE_BASIC_FILE:
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return SQSH_FILE_TYPE_FILE;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return SQSH_FILE_TYPE_SYMLINK;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
		return SQSH_FILE_TYPE_BLOCK;
	case SQSH_INODE_TYPE_BASIC_CHAR:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return SQSH_FILE_TYPE_CHAR;
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
		return SQSH_FILE_TYPE_FIFO;
	case SQSH_INODE_TYPE_BASIC_SOCKET:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return SQSH_FILE_TYPE_SOCKET;
	}
	return SQSH_FILE_TYPE_UNKNOWN;
}

const char *
sqsh_file_symlink(const struct SqshFile *context) {
	const struct SqshDataInodeSymlink *basic;
	const struct SqshDataInodeSymlinkExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		basic = sqsh__data_inode_symlink(inode);
		return (const char *)sqsh__data_inode_symlink_target_path(basic);
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = sqsh__data_inode_symlink_ext(inode);
		return (const char *)sqsh__data_inode_symlink_ext_target_path(extended);
	}
	return NULL;
}

char *
sqsh_file_symlink_dup(const struct SqshFile *inode) {
	const size_t size = sqsh_file_symlink_size(inode);
	const char *link_target = sqsh_file_symlink(inode);

	return cx_memdup(link_target, size);
}

uint32_t
sqsh_file_symlink_size(const struct SqshFile *context) {
	const struct SqshDataInodeSymlink *basic;
	const struct SqshDataInodeSymlinkExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		basic = sqsh__data_inode_symlink(inode);
		return sqsh__data_inode_symlink_target_size(basic);
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = sqsh__data_inode_symlink_ext(inode);
		return sqsh__data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

uint32_t
sqsh_file_device_id(const struct SqshFile *context) {
	const struct SqshDataInodeDevice *basic;
	const struct SqshDataInodeDeviceExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		basic = sqsh__data_inode_device(inode);
		return sqsh__data_inode_device_device(basic);
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		extended = sqsh__data_inode_device_ext(inode);
		return sqsh__data_inode_device_ext_device(extended);
	}
	return 0;
}

static uint32_t
inode_get_id(const struct SqshFile *context, sqsh_index_t idx) {
	int rv = 0;
	struct SqshIdTable *id_table;
	uint32_t id;

	rv = sqsh_archive_id_table(context->archive, &id_table);
	if (rv < 0) {
		return UINT32_MAX;
	}

	rv = sqsh_id_table_get(id_table, idx, &id);
	if (rv < 0) {
		return UINT32_MAX;
	}
	return id;
}

uint32_t
sqsh_file_uid(const struct SqshFile *context) {
	return inode_get_id(context, sqsh__data_inode_uid_idx(get_inode(context)));
}

uint32_t
sqsh_file_gid(const struct SqshFile *context) {
	return inode_get_id(context, sqsh__data_inode_gid_idx(get_inode(context)));
}

uint64_t
sqsh_file_inode_ref(const struct SqshFile *context) {
	return context->inode_ref;
}

uint32_t
sqsh_file_xattr_index(const struct SqshFile *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh__data_inode_type(inode)) {
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return sqsh__data_inode_directory_ext_xattr_idx(
				sqsh__data_inode_directory_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return sqsh__data_inode_file_ext_xattr_idx(
				sqsh__data_inode_file_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return sqsh__data_inode_symlink_ext_xattr_idx(
				sqsh__data_inode_symlink_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return sqsh__data_inode_device_ext_xattr_idx(
				sqsh__data_inode_device_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return sqsh__data_inode_ipc_ext_xattr_idx(
				sqsh__data_inode_ipc_ext(inode));
	}
	return SQSH_INODE_NO_XATTR;
}

int
sqsh__file_cleanup(struct SqshFile *inode) {
	return sqsh__metablock_reader_cleanup(&inode->metablock);
}

struct SqshFile *
sqsh_open(struct SqshArchive *archive, const char *path, int *err) {
	int rv;
	struct SqshPathResolver resolver = {0};
	struct SqshFile *inode = NULL;
	rv = sqsh__path_resolver_init(&resolver, archive);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_path_resolver_resolve(&resolver, path, true);
	if (rv < 0) {
		goto out;
	}

	inode = sqsh_path_resolver_open_file(&resolver, &rv);
	if (rv < 0) {
		goto out;
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	sqsh__path_resolver_cleanup(&resolver);
	return inode;
}

int
sqsh_close(struct SqshFile *file) {
	if (file == NULL) {
		return 0;
	}
	int rv = sqsh__file_cleanup(file);
	free(file);
	return rv;
}
