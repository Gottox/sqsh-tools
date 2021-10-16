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
 * @file        : inode_internal
 * @created     : Wednesday Sep 08, 2021 13:30:05 CEST
 */

#include "../utils.h"
#include "inode.h"

#ifndef FORMAT_INODE_INTERNAL_H

#define FORMAT_INODE_INTERNAL_H

struct SquashInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};
STATIC_ASSERT(
		sizeof(struct SquashInodeDirectoryIndex) ==
		SQUASH_SIZEOF_INODE_DIRECTORY_INDEX);

struct SquashInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};
STATIC_ASSERT(
		sizeof(struct SquashInodeDirectory) == SQUASH_SIZEOF_INODE_DIRECTORY);

struct SquashInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_start;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	// struct SquashInodeDirectoryIndex index[0]; // [index_count]
};
STATIC_ASSERT(
		sizeof(struct SquashInodeDirectoryExt) ==
		SQUASH_SIZEOF_INODE_DIRECTORY_EXT);

struct SquashInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	// uint32_t block_sizes[0];
};
STATIC_ASSERT(sizeof(struct SquashInodeFile) == SQUASH_SIZEOF_INODE_FILE);

struct SquashInodeFileExt {
	uint64_t blocks_start;
	uint64_t file_size;
	uint64_t sparse;
	uint32_t hard_link_count;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t xattr_idx;
	// uint32_t block_sizes[0];
};
STATIC_ASSERT(
		sizeof(struct SquashInodeFileExt) == SQUASH_SIZEOF_INODE_FILE_EXT);

struct SquashInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
STATIC_ASSERT(sizeof(struct SquashInodeSymlink) == SQUASH_SIZEOF_INODE_SYMLINK);

struct SquashInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
STATIC_ASSERT(
		sizeof(struct SquashInodeSymlinkExt) ==
		SQUASH_SIZEOF_INODE_SYMLINK_EXT);

struct SquashInodeSymlinkExtTail {
	uint32_t xattr_idx;
};
STATIC_ASSERT(
		sizeof(struct SquashInodeSymlinkExtTail) ==
		SQUASH_SIZEOF_INODE_SYMLINK_EXT_TAIL);

struct SquashInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};
STATIC_ASSERT(sizeof(struct SquashInodeDevice) == SQUASH_SIZEOF_INODE_DEVICE);

struct SquashInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};
STATIC_ASSERT(
		sizeof(struct SquashInodeDeviceExt) == SQUASH_SIZEOF_INODE_DEVICE_EXT);

struct SquashInodeIpc {
	uint32_t hard_link_count;
};
STATIC_ASSERT(sizeof(struct SquashInodeIpc) == SQUASH_SIZEOF_INODE_IPC);

struct SquashInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};
STATIC_ASSERT(sizeof(struct SquashInodeIpcExt) == SQUASH_SIZEOF_INODE_IPC_EXT);

struct SquashInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};
STATIC_ASSERT(sizeof(struct SquashInodeHeader) == SQUASH_SIZEOF_INODE_HEADER);

struct SquashInode {
	struct SquashInodeHeader header;
	union {
		struct SquashInodeDirectory directory;
		struct SquashInodeDirectoryExt directory_ext;
		struct SquashInodeFile file;
		struct SquashInodeFileExt file_ext;
		struct SquashInodeSymlink symlink;
		struct SquashInodeSymlinkExt symlink_ext;
		struct SquashInodeDevice device;
		struct SquashInodeDeviceExt device_ext;
		struct SquashInodeIpc ipc;
		struct SquashInodeIpcExt ipc_ext;
	} data;
};

#endif /* end of include guard FORMAT_INODE_INTERNAL_H */
