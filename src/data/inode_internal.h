/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         inode_internal.h
 */

#include "inode.h"

#ifndef FORMAT_INODE_INTERNAL_H

#define FORMAT_INODE_INTERNAL_H

struct SQSH_UNALIGNED SqshInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};
STATIC_ASSERT(
		sizeof(struct SqshInodeDirectoryIndex) ==
		SQSH_SIZEOF_INODE_DIRECTORY_INDEX);

struct SQSH_UNALIGNED SqshInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};
STATIC_ASSERT(sizeof(struct SqshInodeDirectory) == SQSH_SIZEOF_INODE_DIRECTORY);

struct SQSH_UNALIGNED SqshInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_start;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	// struct SqshInodeDirectoryIndex index[0]; // [index_count]
};
STATIC_ASSERT(
		sizeof(struct SqshInodeDirectoryExt) ==
		SQSH_SIZEOF_INODE_DIRECTORY_EXT);

struct SQSH_UNALIGNED SqshInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	// uint32_t block_sizes[0];
};
STATIC_ASSERT(sizeof(struct SqshInodeFile) == SQSH_SIZEOF_INODE_FILE);

struct SQSH_UNALIGNED SqshInodeFileExt {
	uint64_t blocks_start;
	uint64_t file_size;
	uint64_t sparse;
	uint32_t hard_link_count;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t xattr_idx;
	// uint32_t block_sizes[0];
};
STATIC_ASSERT(sizeof(struct SqshInodeFileExt) == SQSH_SIZEOF_INODE_FILE_EXT);

struct SQSH_UNALIGNED SqshInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
STATIC_ASSERT(sizeof(struct SqshInodeSymlink) == SQSH_SIZEOF_INODE_SYMLINK);

struct SQSH_UNALIGNED SqshInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
STATIC_ASSERT(
		sizeof(struct SqshInodeSymlinkExt) == SQSH_SIZEOF_INODE_SYMLINK_EXT);

struct SQSH_UNALIGNED SqshInodeSymlinkExtTail {
	uint32_t xattr_idx;
};
STATIC_ASSERT(
		sizeof(struct SqshInodeSymlinkExtTail) ==
		SQSH_SIZEOF_INODE_SYMLINK_EXT_TAIL);

struct SQSH_UNALIGNED SqshInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};
STATIC_ASSERT(sizeof(struct SqshInodeDevice) == SQSH_SIZEOF_INODE_DEVICE);

struct SQSH_UNALIGNED SqshInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};
STATIC_ASSERT(
		sizeof(struct SqshInodeDeviceExt) == SQSH_SIZEOF_INODE_DEVICE_EXT);

struct SQSH_UNALIGNED SqshInodeIpc {
	uint32_t hard_link_count;
};
STATIC_ASSERT(sizeof(struct SqshInodeIpc) == SQSH_SIZEOF_INODE_IPC);

struct SQSH_UNALIGNED SqshInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};
STATIC_ASSERT(sizeof(struct SqshInodeIpcExt) == SQSH_SIZEOF_INODE_IPC_EXT);

struct SQSH_UNALIGNED SqshInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};
STATIC_ASSERT(sizeof(struct SqshInodeHeader) == SQSH_SIZEOF_INODE_HEADER);

struct SQSH_UNALIGNED SqshInode {
	struct SqshInodeHeader header;
	union {
		struct SqshInodeDirectory directory;
		struct SqshInodeDirectoryExt directory_ext;
		struct SqshInodeFile file;
		struct SqshInodeFileExt file_ext;
		struct SqshInodeSymlink symlink;
		struct SqshInodeSymlinkExt symlink_ext;
		struct SqshInodeDevice device;
		struct SqshInodeDeviceExt device_ext;
		struct SqshInodeIpc ipc;
		struct SqshInodeIpcExt ipc_ext;
	} data;
};

#endif /* end of include guard FORMAT_INODE_INTERNAL_H */
