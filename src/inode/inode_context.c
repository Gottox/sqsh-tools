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
 * @file         inode.c
 */

#include "../../include/sqsh.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode_private.h"
#include "../../include/sqsh_iterator.h"
#include "../../include/sqsh_table.h"
#include "../utils.h"

#include <stdint.h>
#include <string.h>

static const struct SqshDataInode *
get_inode(const struct SqshInodeContext *inode) {
	return (const struct SqshDataInode *)sqsh__metablock_stream_data(
			&inode->metablock);
}

static int
inode_data_more(struct SqshInodeContext *inode, size_t size) {
	int rv = sqsh__metablock_stream_more(&inode->metablock, size);

	if (rv < 0) {
		return rv;
	}
	return rv;
}

static int
inode_load(struct SqshInodeContext *context) {
	int rv = 0;
	size_t size = SQSH_SIZEOF_INODE_HEADER;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		size += SQSH_SIZEOF_INODE_DIRECTORY;
		break;
	case SQSH_INODE_TYPE_BASIC_FILE:
		size += SQSH_SIZEOF_INODE_FILE_EXT;
		break;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		size += SQSH_SIZEOF_INODE_SYMLINK;
		break;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		size += SQSH_SIZEOF_INODE_DEVICE;
		break;
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		size += SQSH_SIZEOF_INODE_IPC;
		break;
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		size += SQSH_SIZEOF_INODE_DIRECTORY_EXT;
		break;
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		size += SQSH_SIZEOF_INODE_FILE_EXT;
		break;
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		size += SQSH_SIZEOF_INODE_SYMLINK_EXT;
		break;
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		size += SQSH_SIZEOF_INODE_DEVICE_EXT;
		break;
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		size += SQSH_SIZEOF_INODE_IPC_EXT;
		break;
	}
	rv = inode_data_more(context, size);
	return rv;
}

static const struct SqshDataDatablockSize *
get_size_info(const struct SqshInodeContext *context, int index) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh_data_inode_file(inode);
		return &sqsh_data_inode_file_block_sizes(basic_file)[index];
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh_data_inode_file_ext(inode);
		return &sqsh_data_inode_file_ext_block_sizes(extended_file)[index];
	}
	// Should never happen
	abort();
}

int
sqsh__inode_init(
		struct SqshInodeContext *inode, struct Sqsh *sqsh, uint64_t inode_ref) {
	uint32_t inode_block;
	uint16_t inode_offset;

	sqsh_inode_ref_to_block(inode_ref, &inode_block, &inode_offset);

	int rv = 0;
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	rv = sqsh__metablock_stream_init(
			&inode->metablock, sqsh,
			sqsh_superblock_inode_table_start(superblock), ~0);
	if (rv < 0) {
		return rv;
	}
	rv = sqsh__metablock_stream_seek(
			&inode->metablock, inode_block, inode_offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, SQSH_SIZEOF_INODE_HEADER);
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);
	if (rv < 0) {
		return rv;
	}

	inode->sqsh = sqsh;

	return rv;
}

struct SqshInodeContext *
sqsh_inode_new(struct Sqsh *sqsh, uint64_t inode_ref, int *err) {
	struct SqshInodeContext *context =
			calloc(1, sizeof(struct SqshInodeContext));
	if (context == NULL) {
		return NULL;
	}
	*err = sqsh__inode_init(context, sqsh, inode_ref);
	if (*err < 0) {
		free(context);
		return NULL;
	}
	return context;
}

bool
sqsh_inode_is_extended(const struct SqshInodeContext *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
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
sqsh_inode_hard_link_count(const struct SqshInodeContext *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		return sqsh_data_inode_directory_hard_link_count(
				sqsh_data_inode_directory(inode));
	case SQSH_INODE_TYPE_BASIC_FILE:
		return 1;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		return sqsh_data_inode_symlink_hard_link_count(
				sqsh_data_inode_symlink(inode));
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		return sqsh_data_inode_device_hard_link_count(
				sqsh_data_inode_device(inode));
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		return sqsh_data_inode_ipc_hard_link_count(sqsh_data_inode_ipc(inode));

	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return sqsh_data_inode_directory_ext_hard_link_count(
				sqsh_data_inode_directory_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return sqsh_data_inode_file_ext_hard_link_count(
				sqsh_data_inode_file_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return sqsh_data_inode_symlink_ext_hard_link_count(
				sqsh_data_inode_symlink_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return sqsh_data_inode_device_ext_hard_link_count(
				sqsh_data_inode_device_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return sqsh_data_inode_ipc_ext_hard_link_count(
				sqsh_data_inode_ipc_ext(inode));
	}
	return -SQSH_ERROR_UNKOWN_INODE_TYPE;
}

uint64_t
sqsh_inode_file_size(const struct SqshInodeContext *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;
	const struct SqshDataInodeDirectory *basic_dir;
	const struct SqshDataInodeDirectoryExt *extended_dir;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = sqsh_data_inode_directory(inode);
		return sqsh_data_inode_directory_file_size(basic_dir);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = sqsh_data_inode_directory_ext(inode);
		return sqsh_data_inode_directory_ext_file_size(extended_dir);
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh_data_inode_file(inode);
		return sqsh_data_inode_file_size(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh_data_inode_file_ext(inode);
		return sqsh_data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
sqsh_inode_permission(const struct SqshInodeContext *inode) {
	return sqsh_data_inode_permissions(get_inode(inode));
}

uint32_t
sqsh_inode_number(const struct SqshInodeContext *inode) {
	return sqsh_data_inode_number(get_inode(inode));
}

uint32_t
sqsh_inode_modified_time(const struct SqshInodeContext *inode) {
	return sqsh_data_inode_modified_time(get_inode(inode));
}

uint64_t
sqsh_inode_file_blocks_start(const struct SqshInodeContext *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh_data_inode_file(inode);
		return sqsh_data_inode_file_blocks_start(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh_data_inode_file_ext(inode);
		return sqsh_data_inode_file_ext_blocks_start(extended_file);
	}
	return UINT64_MAX;
}

uint32_t
sqsh_inode_file_block_count(const struct SqshInodeContext *context) {
	struct SqshSuperblockContext *superblock = sqsh_superblock(context->sqsh);
	uint64_t file_size = sqsh_inode_file_size(context);
	uint32_t block_size = sqsh_superblock_block_size(superblock);

	if (file_size == UINT64_MAX) {
		return UINT32_MAX;
	} else if (file_size == 0) {
		return 0;
	} else if (sqsh_inode_file_has_fragment(context)) {
		return file_size / block_size;
	} else {
		return SQSH_DEVIDE_CEIL(file_size, block_size);
	}
}

uint32_t
sqsh_inode_file_block_size(
		const struct SqshInodeContext *inode, uint32_t index) {
	const struct SqshDataDatablockSize *size_info = get_size_info(inode, index);

	return sqsh_data_datablock_size(size_info);
}

bool
sqsh_inode_file_block_is_compressed(
		const struct SqshInodeContext *inode, int index) {
	const struct SqshDataDatablockSize *size_info = get_size_info(inode, index);

	return sqsh_data_datablock_is_compressed(size_info);
}

uint32_t
sqsh_inode_file_fragment_block_index(const struct SqshInodeContext *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh_data_inode_file(inode);
		return sqsh_data_inode_file_fragment_block_index(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh_data_inode_file_ext(inode);
		return sqsh_data_inode_file_ext_fragment_block_index(extended_file);
	}
	return SQSH_INODE_NO_FRAGMENT;
}

uint32_t
sqsh_inode_directory_block_start(const struct SqshInodeContext *context) {
	const struct SqshDataInodeDirectory *basic_file;
	const struct SqshDataInodeDirectoryExt *extended_file;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_file = sqsh_data_inode_directory(inode);
		return sqsh_data_inode_directory_block_start(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_file = sqsh_data_inode_directory_ext(inode);
		return sqsh_data_inode_directory_ext_block_start(extended_file);
	}
	return UINT32_MAX;
}

uint32_t
sqsh_inode_directory_block_offset(const struct SqshInodeContext *context) {
	const struct SqshDataInodeDirectory *basic_directory;
	const struct SqshDataInodeDirectoryExt *extended_directory;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		basic_directory = sqsh_data_inode_directory(inode);
		return sqsh_data_inode_directory_block_offset(basic_directory);
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_directory = sqsh_data_inode_directory_ext(inode);
		return sqsh_data_inode_directory_ext_block_offset(extended_directory);
	}
	return UINT32_MAX;
}

uint32_t
sqsh_inode_file_fragment_block_offset(const struct SqshInodeContext *context) {
	const struct SqshDataInodeFile *basic_file;
	const struct SqshDataInodeFileExt *extended_file;
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_FILE:
		basic_file = sqsh_data_inode_file(inode);
		return sqsh_data_inode_file_block_offset(basic_file);
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		extended_file = sqsh_data_inode_file_ext(inode);
		return sqsh_data_inode_file_ext_block_offset(extended_file);
	}
	return SQSH_INODE_NO_FRAGMENT;
}

bool
sqsh_inode_file_has_fragment(const struct SqshInodeContext *inode) {
	return sqsh_inode_file_fragment_block_index(inode) !=
			SQSH_INODE_NO_FRAGMENT;
}

enum SqshInodeContextType
sqsh_inode_type(const struct SqshInodeContext *context) {
	const struct SqshDataInode *inode = get_inode(context);

	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return SQSH_INODE_TYPE_DIRECTORY;
	case SQSH_INODE_TYPE_BASIC_FILE:
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return SQSH_INODE_TYPE_FILE;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return SQSH_INODE_TYPE_SYMLINK;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
		return SQSH_INODE_TYPE_BLOCK;
	case SQSH_INODE_TYPE_BASIC_CHAR:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return SQSH_INODE_TYPE_CHAR;
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
		return SQSH_INODE_TYPE_FIFO;
	case SQSH_INODE_TYPE_BASIC_SOCKET:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return SQSH_INODE_TYPE_SOCKET;
	}
	return SQSH_INODE_TYPE_UNKNOWN;
}

const char *
sqsh_inode_symlink(const struct SqshInodeContext *context) {
	const struct SqshDataInodeSymlink *basic;
	const struct SqshDataInodeSymlinkExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		basic = sqsh_data_inode_symlink(inode);
		return (const char *)sqsh_data_inode_symlink_target_path(basic);
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = sqsh_data_inode_symlink_ext(inode);
		return (const char *)sqsh_data_inode_symlink_ext_target_path(extended);
	}
	return NULL;
}

int
sqsh_inode_symlink_dup(
		const struct SqshInodeContext *inode, char **namebuffer) {
	int size = sqsh_inode_symlink_size(inode);
	const char *link_target = sqsh_inode_symlink(inode);

	*namebuffer = sqsh_memdup(link_target, size);
	if (*namebuffer) {
		return size;
	} else {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
}

uint32_t
sqsh_inode_symlink_size(const struct SqshInodeContext *context) {
	const struct SqshDataInodeSymlink *basic;
	const struct SqshDataInodeSymlinkExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		basic = sqsh_data_inode_symlink(inode);
		return sqsh_data_inode_symlink_target_size(basic);
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = sqsh_data_inode_symlink_ext(inode);
		return sqsh_data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

uint32_t
sqsh_inode_device_id(const struct SqshInodeContext *context) {
	const struct SqshDataInodeDevice *basic;
	const struct SqshDataInodeDeviceExt *extended;

	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		basic = sqsh_data_inode_device(inode);
		return sqsh_data_inode_device_device(basic);
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		extended = sqsh_data_inode_device_ext(inode);
		return sqsh_data_inode_device_ext_device(extended);
	}
	return 0;
}

static uint32_t
inode_get_id(const struct SqshInodeContext *context, sqsh_index_t idx) {
	int rv = 0;
	struct SqshTable *id_table;
	uint32_t id;

	rv = sqsh_id_table(context->sqsh, &id_table);
	if (rv < 0) {
		return UINT32_MAX;
	}

	rv = sqsh_table_get(id_table, idx, &id);
	if (rv < 0) {
		return UINT32_MAX;
	}
	return id;
}

uint32_t
sqsh_inode_uid(const struct SqshInodeContext *context) {
	return inode_get_id(context, sqsh_data_inode_uid_idx(get_inode(context)));
}

uint32_t
sqsh_inode_gid(const struct SqshInodeContext *context) {
	return inode_get_id(context, sqsh_data_inode_gid_idx(get_inode(context)));
}

uint32_t
sqsh_inode_xattr_index(const struct SqshInodeContext *context) {
	const struct SqshDataInode *inode = get_inode(context);
	switch (sqsh_data_inode_type(inode)) {
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		return sqsh_data_inode_directory_ext_xattr_idx(
				sqsh_data_inode_directory_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		return sqsh_data_inode_file_ext_xattr_idx(
				sqsh_data_inode_file_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		return sqsh_data_inode_symlink_ext_xattr_idx(
				sqsh_data_inode_symlink_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		return sqsh_data_inode_device_ext_xattr_idx(
				sqsh_data_inode_device_ext(inode));
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		return sqsh_data_inode_ipc_ext_xattr_idx(
				sqsh_data_inode_ipc_ext(inode));
	}
	return SQSH_INODE_NO_XATTR;
}

int
sqsh__inode_cleanup(struct SqshInodeContext *inode) {
	return sqsh__metablock_stream_cleanup(&inode->metablock);
}

int
sqsh_inode_free(struct SqshInodeContext *context) {
	if (context == NULL) {
		return 0;
	}
	int rv = sqsh__inode_cleanup(context);
	free(context);
	return rv;
}

void
sqsh_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset) {
	*block_index = (ref & 0x0000FFFFFFFF0000) >> 16;
	*offset = ref & 0x000000000000FFFF;
}
uint64_t
sqsh_inode_ref_from_block(uint32_t block_index, uint16_t offset) {
	return ((uint64_t)block_index << 16) | offset;
}
