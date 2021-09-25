/*
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Wednesday Sep 08, 2021 12:59:19 CEST
 */

#include "datablock.h"
#include <stdint.h>

#ifndef SQUASHFS_FORMAT_INODE_H

#define SQUASHFS_FORMAT_INODE_H

#define SQUASH_SIZEOF_INODE_DIRECTORY_INDEX 12
#define SQUASH_SIZEOF_INODE_DIRECTORY 16
#define SQUASH_SIZEOF_INODE_DIRECTORY_EXT 24
#define SQUASH_SIZEOF_INODE_FILE 16
#define SQUASH_SIZEOF_INODE_FILE_EXT 40
#define SQUASH_SIZEOF_INODE_SYMLINK 8
#define SQUASH_SIZEOF_INODE_SYMLINK_EXT 8
#define SQUASH_SIZEOF_INODE_SYMLINK_EXT_TAIL 4
#define SQUASH_SIZEOF_INODE_DEVICE 8
#define SQUASH_SIZEOF_INODE_DEVICE_EXT 12
#define SQUASH_SIZEOF_INODE_IPC 4
#define SQUASH_SIZEOF_INODE_IPC_EXT 8
#define SQUASH_SIZEOF_INODE_HEADER 16

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

struct SquashInodeDirectory;
struct SquashInodeDirectoryExt;
struct SquashInodeFile;
struct SquashInodeFileExt;
struct SquashInodeSymlink;
struct SquashInodeSymlinkExt;
struct SquashInodeSymlinkExtTail;
struct SquashInodeDevice;
struct SquashInodeDeviceExt;
struct SquashInodeIpc;
struct SquashInodeIpcExt;
struct SquashInode;

struct SquashInodeDirectoryIndex;

uint16_t squash_data_inode_type(const struct SquashInode *inode);
uint16_t squash_data_inode_permissions(const struct SquashInode *inode);
uint16_t squash_data_inode_uid_idx(const struct SquashInode *inode);
uint16_t squash_data_inode_gid_idx(const struct SquashInode *inode);
uint32_t squash_data_inode_modified_time(const struct SquashInode *inode);
uint32_t squash_data_inode_number(const struct SquashInode *inode);

const struct SquashInodeDirectory *squash_data_inode_directory(
		const struct SquashInode *inode);
const struct SquashInodeDirectoryExt *squash_data_inode_directory_ext(
		const struct SquashInode *inode);
const struct SquashInodeFile *squash_data_inode_file(
		const struct SquashInode *inode);
const struct SquashInodeFileExt *squash_data_inode_file_ext(
		const struct SquashInode *inode);
const struct SquashInodeSymlink *squash_data_inode_symlink(
		const struct SquashInode *inode);
const struct SquashInodeSymlinkExt *squash_data_inode_symlink_ext(
		const struct SquashInode *inode);
const struct SquashInodeDevice *squash_data_inode_device(
		const struct SquashInode *inode);
const struct SquashInodeDeviceExt *squash_data_inode_device_ext(
		const struct SquashInode *inode);
const struct SquashInodeIpc *squash_data_inode_ipc(
		const struct SquashInode *inode);
const struct SquashInodeIpcExt *squash_data_inode_ipc_ext(
		const struct SquashInode *inode);

uint32_t squash_data_inode_directory_block_start(
		const struct SquashInodeDirectory *directory);
uint32_t squash_data_inode_directory_hard_link_count(
		const struct SquashInodeDirectory *directory);
uint16_t squash_data_inode_directory_file_size(
		const struct SquashInodeDirectory *directory);
uint16_t squash_data_inode_directory_block_offset(
		const struct SquashInodeDirectory *directory);
uint32_t squash_data_inode_directory_parent_inode_number(
		const struct SquashInodeDirectory *directory);

uint32_t squash_data_inode_directory_ext_hard_link_count(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_data_inode_directory_ext_file_size(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_data_inode_directory_ext_block_start(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_data_inode_directory_ext_parent_inode_number(
		const struct SquashInodeDirectoryExt *directory_ext);
uint16_t squash_data_inode_directory_ext_index_count(
		const struct SquashInodeDirectoryExt *directory_ext);
uint16_t squash_data_inode_directory_ext_block_offset(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_data_inode_directory_ext_xattr_idx(
		const struct SquashInodeDirectoryExt *directory_ext);
const struct SquashInodeDirectoryIndex *squash_data_inode_directory_ext_index(
		const struct SquashInodeDirectoryExt *directory_ext);

uint32_t squash_data_inode_directory_index_index(
		const struct SquashInodeDirectoryIndex *directory_index);
uint32_t squash_data_inode_directory_index_start(
		const struct SquashInodeDirectoryIndex *directory_index);
uint32_t squash_data_inode_directory_index_name_size(
		const struct SquashInodeDirectoryIndex *directory_index);
const uint8_t *squash_data_inode_directory_index_name(
		const struct SquashInodeDirectoryIndex *directory_index);

uint32_t squash_data_inode_file_blocks_start(
		const struct SquashInodeFile *file);
uint32_t squash_data_inode_file_fragment_block_index(
		const struct SquashInodeFile *file);
uint32_t squash_data_inode_file_block_offset(
		const struct SquashInodeFile *file);
uint32_t squash_data_inode_file_size(const struct SquashInodeFile *file);
const struct SquashDatablockSize *squash_data_inode_file_block_sizes(
		const struct SquashInodeFile *file);

uint64_t squash_data_inode_file_ext_blocks_start(
		const struct SquashInodeFileExt *file_ext);
uint64_t squash_data_inode_file_ext_size(
		const struct SquashInodeFileExt *file_ext);
uint64_t squash_data_inode_file_ext_sparse(
		const struct SquashInodeFileExt *file_ext);
uint32_t squash_data_inode_file_ext_hard_link_count(
		const struct SquashInodeFileExt *file_ext);
uint32_t squash_data_inode_file_ext_fragment_block_index(
		const struct SquashInodeFileExt *file_ext);
uint32_t squash_data_inode_file_ext_block_offset(
		const struct SquashInodeFileExt *file_ext);
uint32_t squash_data_inode_file_ext_xattr_idx(
		const struct SquashInodeFileExt *file_ext);
const struct SquashDatablockSize *squash_data_inode_file_ext_block_sizes(
		const struct SquashInodeFileExt *file_ext);

uint32_t squash_data_inode_symlink_hard_link_count(
		const struct SquashInodeSymlink *directory);
uint32_t squash_data_inode_symlink_target_size(
		const struct SquashInodeSymlink *directory);
const uint8_t *squash_data_inode_symlink_target_path(
		const struct SquashInodeSymlink *directory);

uint32_t squash_data_inode_symlink_ext_hard_link_count(
		const struct SquashInodeSymlinkExt *directory);
uint32_t squash_data_inode_symlink_ext_target_size(
		const struct SquashInodeSymlinkExt *directory);
const uint8_t *squash_data_inode_symlink_ext_target_path(
		const struct SquashInodeSymlinkExt *directory);
uint32_t squash_data_inode_symlink_ext_xattr_idx(
		const struct SquashInodeSymlinkExt *directory);

uint32_t squash_data_inode_device_hard_link_count(
		const struct SquashInodeDevice *device);
uint32_t squash_data_inode_device_device(
		const struct SquashInodeDevice *device);

uint32_t squash_data_inode_device_ext_hard_link_count(
		const struct SquashInodeDeviceExt *device);
uint32_t squash_data_inode_device_ext_device(
		const struct SquashInodeDeviceExt *device);
uint32_t squash_data_inode_device_ext_xattr_idx(
		const struct SquashInodeDeviceExt *device);

uint32_t squash_data_inode_ipc_hard_link_count(
		const struct SquashInodeIpc *ipc);

uint32_t squash_data_inode_ipc_ext_hard_link_count(
		const struct SquashInodeIpcExt *ipc);
uint32_t squash_data_inode_ipc_ext_xattr_idx(
		const struct SquashInodeIpcExt *ipc);

#endif /* end of include guard SQUASHFS_FORMAT_INODE_H */
