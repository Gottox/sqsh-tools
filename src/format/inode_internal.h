/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode_internal
 * @created     : Wednesday Sep 08, 2021 13:30:05 CEST
 */

#include "inode.h"
#include <stdint.h>

#ifndef FORMAT_INODE_INTERNAL_H

#define FORMAT_INODE_INTERNAL_H

struct SquashInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};

struct SquashInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};

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

struct SquashInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	// uint32_t block_sizes[0];
};

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

struct SquashInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};

struct SquashInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};

struct SquashInodeSymlinkExtTail {
	uint32_t xattr_idx;
};

struct SquashInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};

struct SquashInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};

struct SquashInodeIpc {
	uint32_t hard_link_count;
};

struct SquashInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};

struct SquashInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};

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
