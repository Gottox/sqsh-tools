/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "inode_context.h"
#include "../data/datablock_internal.h"
#include "../data/inode.h"

#include "../error.h"
#include "../hsqs.h"
#include "../utils.h"
#include "directory_context.h"
#include "metablock_context.h"
#include <stdint.h>

static const struct HsqsInodeDirectoryIndex *
current_directory_index(struct HsqsInodeDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->inode->inode;
	return (const struct HsqsInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
inode_data_more(struct HsqsInodeContext *inode, size_t size) {
	int rv = hsqs_metablock_more(&inode->extract, size);

	if (rv < 0) {
		return rv;
	}
	inode->inode = (struct HsqsInode *)hsqs_metablock_data(&inode->extract);
	return rv;
}

static int
inode_load(struct HsqsInodeContext *inode) {
	int rv = 0;
	size_t size = HSQS_SIZEOF_INODE_HEADER;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		size += HSQS_SIZEOF_INODE_DIRECTORY;
		break;
	case HSQS_INODE_TYPE_BASIC_FILE:
		size += HSQS_SIZEOF_INODE_FILE_EXT;
		break;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		size += HSQS_SIZEOF_INODE_SYMLINK;
		break;
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
		size += HSQS_SIZEOF_INODE_DEVICE;
		break;
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_BASIC_SOCKET:
		size += HSQS_SIZEOF_INODE_IPC;
		break;
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		size += HSQS_SIZEOF_INODE_DIRECTORY_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		size += HSQS_SIZEOF_INODE_FILE_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		size += HSQS_SIZEOF_INODE_SYMLINK_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		size += HSQS_SIZEOF_INODE_DEVICE_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		size += HSQS_SIZEOF_INODE_IPC_EXT;
		break;
	}
	rv = inode_data_more(inode, size);
	return rv;
}

uint32_t
hsqs_inode_hard_link_count(const struct HsqsInodeContext *inode) {
	struct HsqsInode *wrap = inode->inode;
	switch (hsqs_data_inode_type(wrap)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		return hsqs_data_inode_directory_hard_link_count(
				hsqs_data_inode_directory(wrap));
	case HSQS_INODE_TYPE_BASIC_FILE:
		return 1;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		return hsqs_data_inode_symlink_hard_link_count(
				hsqs_data_inode_symlink(wrap));
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
		return hsqs_data_inode_device_hard_link_count(
				hsqs_data_inode_device(wrap));
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_BASIC_SOCKET:
		return hsqs_data_inode_ipc_hard_link_count(hsqs_data_inode_ipc(wrap));

	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return hsqs_data_inode_directory_ext_hard_link_count(
				hsqs_data_inode_directory_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return hsqs_data_inode_file_ext_hard_link_count(
				hsqs_data_inode_file_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return hsqs_data_inode_symlink_ext_hard_link_count(
				hsqs_data_inode_symlink_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return hsqs_data_inode_device_ext_hard_link_count(
				hsqs_data_inode_device_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return hsqs_data_inode_ipc_ext_hard_link_count(
				hsqs_data_inode_ipc_ext(wrap));
	}
	return -HSQS_ERROR_UNKOWN_INODE_TYPE;
}

uint64_t
hsqs_inode_file_size(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;
	const struct HsqsInodeDirectory *basic_dir;
	const struct HsqsInodeDirectoryExt *extended_dir;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = hsqs_data_inode_directory(inode->inode);
		return hsqs_data_inode_directory_file_size(basic_dir);
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = hsqs_data_inode_directory_ext(inode->inode);
		return hsqs_data_inode_directory_ext_file_size(extended_dir);
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode->inode);
		return hsqs_data_inode_file_size(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode->inode);
		return hsqs_data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
hsqs_inode_permission(const struct HsqsInodeContext *inode) {
	return hsqs_data_inode_permissions(inode->inode);
}

uint32_t
hsqs_inode_modified_time(const struct HsqsInodeContext *inode) {
	return hsqs_data_inode_modified_time(inode->inode);
}

uint64_t
hsqs_inode_file_blocks_start(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode->inode);
		return hsqs_data_inode_file_blocks_start(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode->inode);
		return hsqs_data_inode_file_ext_blocks_start(extended_file);
	}
	// Should never happen
	abort();
	return 0;
}

static const struct HsqsDatablockSize *
get_size_info(const struct HsqsInodeContext *inode, int index) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode->inode);
		return &hsqs_data_inode_file_block_sizes(basic_file)[index];
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode->inode);
		return &hsqs_data_inode_file_ext_block_sizes(extended_file)[index];
	}
	// Should never happen
	abort();
}

uint32_t
hsqs_inode_file_block_size(const struct HsqsInodeContext *inode, int index) {
	const struct HsqsDatablockSize *size_info = get_size_info(inode, index);

	return hsqs_data_datablock_size(size_info);
}

bool
hsqs_inode_file_block_is_compressed(
		const struct HsqsInodeContext *inode, int index) {
	const struct HsqsDatablockSize *size_info = get_size_info(inode, index);

	return hsqs_data_datablock_is_compressed(size_info);
}

uint32_t
hsqs_inode_file_fragment_block_index(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode->inode);
		return hsqs_data_inode_file_fragment_block_index(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode->inode);
		return hsqs_data_inode_file_ext_fragment_block_index(extended_file);
	}
	return HSQS_INODE_NO_FRAGMENT;
}

uint32_t
hsqs_inode_file_fragment_block_offset(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode->inode);
		return hsqs_data_inode_file_block_offset(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode->inode);
		return hsqs_data_inode_file_ext_block_offset(extended_file);
	}
	return HSQS_INODE_NO_FRAGMENT;
}

enum HsqsInodeContextType
hsqs_inode_type(const struct HsqsInodeContext *inode) {
	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return HSQS_INODE_TYPE_DIRECTORY;
	case HSQS_INODE_TYPE_BASIC_FILE:
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return HSQS_INODE_TYPE_FILE;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return HSQS_INODE_TYPE_SYMLINK;
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
		return HSQS_INODE_TYPE_BLOCK;
	case HSQS_INODE_TYPE_BASIC_CHAR:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return HSQS_INODE_TYPE_CHAR;
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
		return HSQS_INODE_TYPE_FIFO;
	case HSQS_INODE_TYPE_BASIC_SOCKET:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return HSQS_INODE_TYPE_SOCKET;
	}
	return HSQS_INODE_TYPE_UNKNOWN;
}

const char *
hsqs_inode_symlink(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeSymlink *basic;
	const struct HsqsInodeSymlinkExt *extended;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		basic = hsqs_data_inode_symlink(inode->inode);
		return (const char *)hsqs_data_inode_symlink_target_path(basic);
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		extended = hsqs_data_inode_symlink_ext(inode->inode);
		return (const char *)hsqs_data_inode_symlink_ext_target_path(extended);
	}
	return NULL;
}

int
hsqs_inode_symlink_dup(
		const struct HsqsInodeContext *inode, char **namebuffer) {
	int size = hsqs_inode_symlink_size(inode);
	const char *link_target = hsqs_inode_symlink(inode);

	*namebuffer = hsqs_memdup(link_target, size);
	if (*namebuffer) {
		return size;
	} else {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
}

uint32_t
hsqs_inode_symlink_size(const struct HsqsInodeContext *inode) {
	const struct HsqsInodeSymlink *basic;
	const struct HsqsInodeSymlinkExt *extended;

	switch (hsqs_data_inode_type(inode->inode)) {
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		basic = hsqs_data_inode_symlink(inode->inode);
		return hsqs_data_inode_symlink_target_size(basic);
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		extended = hsqs_data_inode_symlink_ext(inode->inode);
		return hsqs_data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

int
hsqs_inode_load(
		struct HsqsInodeContext *inode,
		struct HsqsSuperblockContext *superblock, uint64_t inode_ref) {
	uint32_t inode_block;
	uint16_t inode_offset;

	hsqs_inode_ref_to_block(inode_ref, &inode_block, &inode_offset);

	int rv = 0;
	inode->inode = NULL;

	rv = hsqs_metablock_init(
			&inode->extract, superblock,
			hsqs_superblock_inode_table_start(superblock));
	if (rv < 0) {
		return rv;
	}
	rv = hsqs_metablock_seek(&inode->extract, inode_block, inode_offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, HSQS_SIZEOF_INODE_HEADER);
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);
	if (rv < 0) {
		return rv;
	}

	inode->superblock = superblock;

	return rv;
}

static uint32_t
inode_get_id(const struct HsqsInodeContext *context, off_t idx) {
	int rv = 0;
	struct HsqsTableContext *id_table =
			hsqs_superblock_id_table(context->superblock);
	const uint32_t *value;

	rv = hsqs_table_get(id_table, idx, (const void **)&value);
	if (rv < 0) {
		return UINT32_MAX;
	}
	return *value;
}

uint32_t
hsqs_inode_uid(const struct HsqsInodeContext *context) {
	return inode_get_id(context, hsqs_data_inode_uid_idx(context->inode));
}

uint32_t
hsqs_inode_gid(const struct HsqsInodeContext *context) {
	return inode_get_id(context, hsqs_data_inode_gid_idx(context->inode));
}

uint32_t
hsqs_inode_xattr_index(const struct HsqsInodeContext *inode) {
	struct HsqsInode *wrap = inode->inode;
	switch (hsqs_data_inode_type(wrap)) {
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return hsqs_data_inode_directory_ext_xattr_idx(
				hsqs_data_inode_directory_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return hsqs_data_inode_file_ext_xattr_idx(
				hsqs_data_inode_file_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return hsqs_data_inode_symlink_ext_xattr_idx(
				hsqs_data_inode_symlink_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return hsqs_data_inode_device_ext_xattr_idx(
				hsqs_data_inode_device_ext(wrap));
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return hsqs_data_inode_ipc_ext_xattr_idx(hsqs_data_inode_ipc_ext(wrap));
	}
	return HSQS_INODE_NO_XATTR;
}

int
hsqs_inode_cleanup(struct HsqsInodeContext *inode) {
	return hsqs_metablock_cleanup(&inode->extract);
}

int
hsqs_inode_directory_iterator_init(
		struct HsqsInodeDirectoryIndexIterator *iterator,
		struct HsqsInodeContext *inode) {
	int rv = 0;

	if (hsqs_data_inode_type(inode->inode) !=
		HSQS_INODE_TYPE_EXTENDED_DIRECTORY) {
		return -HSQS_ERROR_NO_EXTENDED_DIRECTORY;
	}

	const struct HsqsInodeDirectoryExt *xdir =
			hsqs_data_inode_directory_ext(inode->inode);

	iterator->inode = inode;
	iterator->current_offset = 0;
	iterator->next_offset = HSQS_SIZEOF_INODE_DIRECTORY_EXT;
	iterator->indices = hsqs_data_inode_directory_ext_index(xdir);
	iterator->remaining_entries =
			hsqs_data_inode_directory_ext_index_count(xdir);
	return rv;
}

int
hsqs_inode_directory_index_iterator_next(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	// Make sure next entry is loaded:
	iterator->next_offset += HSQS_SIZEOF_INODE_DIRECTORY_INDEX;
	rv = inode_data_more(iterator->inode, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure current index has its name populated
	iterator->next_offset += hsqs_data_inode_directory_index_name_size(
			current_directory_index(iterator));
	rv = inode_data_more(iterator->inode, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

uint32_t
hsqs_inode_directory_index_iterator_index(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_index(current);
}
uint32_t
hsqs_inode_directory_index_iterator_start(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_start(current);
}
uint32_t
hsqs_inode_directory_index_iterator_name_size(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_name_size(current) + 1;
}
const char *
hsqs_inode_directory_index_iterator_name(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)hsqs_data_inode_directory_index_name(current);
}

int
hsqs_inode_directory_index_iterator_clean(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	return 0;
}

void
hsqs_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset) {
	*block_index = (ref & 0x0000FFFFFFFF0000) >> 16;
	*offset = ref & 0x000000000000FFFF;
}
uint64_t
hsqs_inode_ref_from_block(uint32_t block_index, uint16_t offset) {
	return ((uint64_t)block_index << 16) | offset;
}
