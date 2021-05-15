/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#ifndef INODE_H

#define INODE_H

#include <stdint.h>
#include <stdlib.h>

struct Squash;

enum SquashInodeType {
	SQUASH_INODE_TYPE_BASIC_DIRECTORY = 1,
	SQUASH_INODE_TYPE_BASIC_FILE = 2,
	SQUASH_INODE_TYPE_BASIC_SYMLINK = 3,
	SQUASH_INODE_TYPE_BASIC_BLOCK = 4,
	SQUASH_INODE_TYPE_BASIC_CHAR = 5,
	SQUASH_INODE_TYPE_BASIC_FIFO = 6,
	SQUASH_INODE_TYPE_BASIC_SOCKET = 7,
	SQUASH_INODE_TYPE_EXTENDED_DIRECTORY = 8,
	SQUASH_INODE_TYPE_EXTENDED_FILE = 9,
	SQUASH_INODE_TYPE_EXTENDED_SYMLINK = 10,
	SQUASH_INODE_TYPE_EXTENDED_BLOCK = 11,
	SQUASH_INODE_TYPE_EXTENDED_CHAR = 12,
	SQUASH_INODE_TYPE_EXTENDED_FIFO = 13,
	SQUASH_INODE_TYPE_EXTENDED_SOCKET = 14,
};

struct SquashInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	uint8_t name[0]; // [name_size + 1]
};

struct SquashInodeDirectory {
	uint32_t block_idx;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};

struct SquashInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_idx;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	struct SquashInodeDirectoryIndex dir_index[0]; // [index_count]
};

struct SquashInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	uint32_t block_sizes[0];
};

struct SquashInodeFileExt {
	uint64_t blocks_start;
	uint64_t file_size;
	uint64_t sparse;
	uint32_t hard_link_count;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t xattr_idx;
	uint32_t block_sizes[0];
};

struct SquashInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	uint8_t target_path[0]; // [target_size]
};

struct SquashInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	uint8_t target_path[0]; // [target_size]
	// uint32_t xattr_idx;
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

union SquashInodeData {
	struct SquashInodeDirectory dir;
	struct SquashInodeDirectoryExt xdir;
	struct SquashInodeFile file;
	struct SquashInodeFileExt xfile;
	struct SquashInodeSymlink sym;
	struct SquashInodeSymlinkExt xsym;
	struct SquashInodeDevice dev;
	struct SquashInodeDeviceExt xdev;
	struct SquashInodeIpc ipc;
	struct SquashInodeIpcExt xipc;
};

struct SquashInodeWrap {
	uint16_t inode_type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
	union SquashInodeData data;
};

struct SquashInode {
	struct SquashInodeWrap *wrap;
	struct SquashStream *stream;
};
#endif /* end of include guard INODE_H */
