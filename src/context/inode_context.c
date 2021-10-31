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
#include "../squash.h"
#include "../utils.h"
#include "directory_context.h"
#include "metablock_context.h"
#include <stdint.h>

static const struct SquashInodeDirectoryIndex *
current_directory_index(struct SquashInodeDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->inode->inode;
	return (const struct SquashInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
inode_data_more(struct SquashInodeContext *inode, size_t size) {
	int rv = squash_metablock_more(&inode->extract, size);

	if (rv < 0) {
		return rv;
	}
	inode->inode = (struct SquashInode *)squash_metablock_data(&inode->extract);
	return rv;
}

static int
inode_load(struct SquashInodeContext *inode) {
	int rv = 0;
	size_t size = SQUASH_SIZEOF_INODE_HEADER;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		size += SQUASH_SIZEOF_INODE_DIRECTORY;
		break;
	case SQUASH_INODE_TYPE_BASIC_FILE:
		size += SQUASH_SIZEOF_INODE_FILE_EXT;
		break;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		size += SQUASH_SIZEOF_INODE_SYMLINK;
		break;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		size += SQUASH_SIZEOF_INODE_DEVICE;
		break;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		size += SQUASH_SIZEOF_INODE_IPC;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		size += SQUASH_SIZEOF_INODE_DIRECTORY_EXT;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		size += SQUASH_SIZEOF_INODE_FILE_EXT;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		size += SQUASH_SIZEOF_INODE_SYMLINK_EXT;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		size += SQUASH_SIZEOF_INODE_DEVICE_EXT;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		size += SQUASH_SIZEOF_INODE_IPC_EXT;
		break;
	}
	rv = inode_data_more(inode, size);
	return rv;
}

uint32_t
squash_inode_hard_link_count(const struct SquashInodeContext *inode) {
	struct SquashInode *wrap = inode->inode;
	switch (squash_data_inode_type(wrap)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		return squash_data_inode_directory_hard_link_count(
				squash_data_inode_directory(wrap));
	case SQUASH_INODE_TYPE_BASIC_FILE:
		return 1;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		return squash_data_inode_symlink_hard_link_count(
				squash_data_inode_symlink(wrap));
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		return squash_data_inode_device_hard_link_count(
				squash_data_inode_device(wrap));
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		return squash_data_inode_ipc_hard_link_count(
				squash_data_inode_ipc(wrap));

	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		return squash_data_inode_directory_ext_hard_link_count(
				squash_data_inode_directory_ext(wrap));
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		return squash_data_inode_file_ext_hard_link_count(
				squash_data_inode_file_ext(wrap));
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		return squash_data_inode_symlink_ext_hard_link_count(
				squash_data_inode_symlink_ext(wrap));
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		return squash_data_inode_device_ext_hard_link_count(
				squash_data_inode_device_ext(wrap));
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		return squash_data_inode_ipc_ext_hard_link_count(
				squash_data_inode_ipc_ext(wrap));
	}
	return -SQUASH_ERROR_UNKOWN_INODE_TYPE;
}

uint64_t
squash_inode_file_size(const struct SquashInodeContext *inode) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;
	const struct SquashInodeDirectory *basic_dir;
	const struct SquashInodeDirectoryExt *extended_dir;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = squash_data_inode_directory(inode->inode);
		return squash_data_inode_directory_file_size(basic_dir);
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = squash_data_inode_directory_ext(inode->inode);
		return squash_data_inode_directory_ext_file_size(extended_dir);
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return squash_data_inode_file_size(basic_file);
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return squash_data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
squash_inode_permission(const struct SquashInodeContext *inode) {
	return squash_data_inode_permissions(inode->inode);
}

uint32_t
squash_inode_modified_time(const struct SquashInodeContext *inode) {
	return squash_data_inode_modified_time(inode->inode);
}

uint64_t
squash_inode_file_blocks_start(const struct SquashInodeContext *inode) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return squash_data_inode_file_blocks_start(basic_file);
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return squash_data_inode_file_ext_blocks_start(extended_file);
	}
	// Should never happen
	abort();
	return 0;
}

static const struct SquashDatablockSize *
get_size_info(const struct SquashInodeContext *inode, int index) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return &squash_data_inode_file_block_sizes(basic_file)[index];
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return &squash_data_inode_file_ext_block_sizes(extended_file)[index];
	}
	// Should never happen
	abort();
}

uint32_t
squash_inode_file_block_size(
		const struct SquashInodeContext *inode, int index) {
	const struct SquashDatablockSize *size_info = get_size_info(inode, index);

	return squash_data_datablock_size(size_info);
}

bool
squash_inode_file_block_is_compressed(
		const struct SquashInodeContext *inode, int index) {
	const struct SquashDatablockSize *size_info = get_size_info(inode, index);

	return squash_data_datablock_is_compressed(size_info);
}

uint32_t
squash_inode_file_fragment_block_index(const struct SquashInodeContext *inode) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return squash_data_inode_file_fragment_block_index(basic_file);
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return squash_data_inode_file_ext_fragment_block_index(extended_file);
	}
	return SQUASH_INODE_NO_FRAGMENT;
}

uint32_t
squash_inode_file_fragment_block_offset(
		const struct SquashInodeContext *inode) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return squash_data_inode_file_block_offset(basic_file);
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return squash_data_inode_file_ext_block_offset(extended_file);
	}
	return SQUASH_INODE_NO_FRAGMENT;
}

enum SquashInodeContextType
squash_inode_type(const struct SquashInodeContext *inode) {
	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		return SQUASH_INODE_TYPE_DIRECTORY;
	case SQUASH_INODE_TYPE_BASIC_FILE:
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		return SQUASH_INODE_TYPE_FILE;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		return SQUASH_INODE_TYPE_SYMLINK;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
		return SQUASH_INODE_TYPE_BLOCK;
	case SQUASH_INODE_TYPE_BASIC_CHAR:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		return SQUASH_INODE_TYPE_CHAR;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
		return SQUASH_INODE_TYPE_FIFO;
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		return SQUASH_INODE_TYPE_SOCKET;
	}
	return SQUASH_INODE_TYPE_UNKNOWN;
}

const char *
squash_inode_symlink(const struct SquashInodeContext *inode) {
	const struct SquashInodeSymlink *basic;
	const struct SquashInodeSymlinkExt *extended;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		basic = squash_data_inode_symlink(inode->inode);
		return (const char *)squash_data_inode_symlink_target_path(basic);
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = squash_data_inode_symlink_ext(inode->inode);
		return (const char *)squash_data_inode_symlink_ext_target_path(
				extended);
	}
	return NULL;
}

int
squash_inode_symlink_dup(
		const struct SquashInodeContext *inode, char **namebuffer) {
	int size = squash_inode_symlink_size(inode);
	const char *link_target = squash_inode_symlink(inode);

	*namebuffer = squash_memdup(link_target, size);
	if (*namebuffer) {
		return size;
	} else {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
}

uint32_t
squash_inode_symlink_size(const struct SquashInodeContext *inode) {
	const struct SquashInodeSymlink *basic;
	const struct SquashInodeSymlinkExt *extended;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		basic = squash_data_inode_symlink(inode->inode);
		return squash_data_inode_symlink_target_size(basic);
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = squash_data_inode_symlink_ext(inode->inode);
		return squash_data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

int
squash_inode_load(
		struct SquashInodeContext *inode,
		struct SquashSuperblockContext *superblock, uint64_t inode_ref) {
	uint32_t inode_block;
	uint16_t inode_offset;

	squash_inode_ref_to_block(inode_ref, &inode_block, &inode_offset);

	int rv = 0;
	inode->inode = NULL;

	rv = squash_metablock_init(
			&inode->extract, superblock,
			squash_superblock_inode_table_start(superblock));
	if (rv < 0) {
		return rv;
	}
	rv = squash_metablock_seek(&inode->extract, inode_block, inode_offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, SQUASH_SIZEOF_INODE_HEADER);
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);
	if (rv < 0) {
		return rv;
	}

	inode->datablock_block_size = squash_superblock_block_size(superblock);

	return rv;
}

int
squash_inode_cleanup(struct SquashInodeContext *inode) {
	return squash_metablock_cleanup(&inode->extract);
}

int
squash_inode_directory_iterator_init(
		struct SquashInodeDirectoryIndexIterator *iterator,
		struct SquashInodeContext *inode) {
	int rv = 0;

	if (squash_data_inode_type(inode->inode) !=
		SQUASH_INODE_TYPE_EXTENDED_DIRECTORY) {
		return -SQUASH_ERROR_NO_EXTENDED_DIRECTORY;
	}

	const struct SquashInodeDirectoryExt *xdir =
			squash_data_inode_directory_ext(inode->inode);

	iterator->inode = inode;
	iterator->current_offset = 0;
	iterator->next_offset = SQUASH_SIZEOF_INODE_DIRECTORY_EXT;
	iterator->indices = squash_data_inode_directory_ext_index(xdir);
	iterator->remaining_entries =
			squash_data_inode_directory_ext_index_count(xdir);
	return rv;
}

int
squash_inode_directory_index_iterator_next(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	// Make sure next entry is loaded:
	iterator->next_offset += SQUASH_SIZEOF_INODE_DIRECTORY_INDEX;
	rv = inode_data_more(iterator->inode, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure current index has its name populated
	iterator->next_offset += squash_data_inode_directory_index_name_size(
			current_directory_index(iterator));
	rv = inode_data_more(iterator->inode, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

uint32_t
squash_inode_directory_index_iterator_index(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	const struct SquashInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return squash_data_inode_directory_index_index(current);
}
uint32_t
squash_inode_directory_index_iterator_start(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	const struct SquashInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return squash_data_inode_directory_index_start(current);
}
uint32_t
squash_inode_directory_index_iterator_name_size(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	const struct SquashInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return squash_data_inode_directory_index_name_size(current) + 1;
}
const char *
squash_inode_directory_index_iterator_name(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	const struct SquashInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)squash_data_inode_directory_index_name(current);
}

int
squash_inode_directory_index_iterator_clean(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	return 0;
}

void
squash_inode_ref_to_block(
		uint64_t ref, uint32_t *block_index, uint16_t *offset) {
	*block_index = (ref & 0x0000FFFFFFFF0000) >> 16;
	*offset = ref & 0x000000000000FFFF;
}
uint64_t
squash_inode_ref_from_block(uint32_t block_index, uint16_t offset) {
	return ((uint64_t)block_index << 16) | offset;
}
