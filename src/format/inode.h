/*
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Wednesday Sep 08, 2021 12:59:19 CEST
 */

#include <stdint.h>

#ifndef FORMAT_INODE_H

#define FORMAT_INODE_H

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

uint16_t squash_format_inode_type(const struct SquashInode *inode);
uint16_t squash_format_inode_permissions(const struct SquashInode *inode);
uint16_t squash_format_inode_uid_idx(const struct SquashInode *inode);
uint16_t squash_format_inode_gid_idx(const struct SquashInode *inode);
uint32_t squash_format_inode_modified_time(const struct SquashInode *inode);
uint32_t squash_format_inode_inode_number(const struct SquashInode *inode);

const struct SquashInodeDirectory *squash_format_inode_directory(
		const struct SquashInode *inode);
const struct SquashInodeDirectoryExt *squash_format_inode_directory_ext(
		const struct SquashInode *inode);
const struct SquashInodeFile *squash_format_inode_file(
		const struct SquashInode *inode);
const struct SquashInodeFileExt *squash_format_inode_file_ext(
		const struct SquashInode *inode);
const struct SquashInodeSymlink *squash_format_inode_symlink(
		const struct SquashInode *inode);
const struct SquashInodeSymlinkExt *squash_format_inode_symlink_ext(
		const struct SquashInode *inode);
const struct SquashInodeDevice *squash_format_inode_device(
		const struct SquashInode *inode);
const struct SquashInodeDeviceExt *squash_format_inode_device_ext(
		const struct SquashInode *inode);
const struct SquashInodeIpc *squash_format_inode_ipc(
		const struct SquashInode *inode);
const struct SquashInodeIpcExt *squash_format_inode_ipc_ext(
		const struct SquashInode *inode);

uint32_t squash_format_inode_directory_block_start(
		const struct SquashInodeDirectory *directory);
uint32_t squash_format_inode_directory_hard_link_count(
		const struct SquashInodeDirectory *directory);
uint16_t squash_format_inode_directory_file_size(
		const struct SquashInodeDirectory *directory);
uint16_t squash_format_inode_directory_block_offset(
		const struct SquashInodeDirectory *directory);
uint32_t squash_format_inode_directory_parent_inode_number(
		const struct SquashInodeDirectory *directory);

uint32_t squash_format_inode_directory_ext_hard_link_count(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_format_inode_directory_ext_file_size(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_format_inode_directory_ext_dir_block_start(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_format_inode_directory_ext_parent_inode_number(
		const struct SquashInodeDirectoryExt *directory_ext);
uint16_t squash_format_inode_directory_ext_index_count(
		const struct SquashInodeDirectoryExt *directory_ext);
uint16_t squash_format_inode_directory_ext_block_offset(
		const struct SquashInodeDirectoryExt *directory_ext);
uint32_t squash_format_inode_directory_ext_xattr_idx(
		const struct SquashInodeDirectoryExt *directory_ext);
const struct SquashInodeDirectoryIndex *squash_format_inode_directory_ext_index(
		const struct SquashInodeDirectoryExt *directory_ext);

uint32_t squash_format_inode_directory_index_index(
		const struct SquashInodeDirectoryIndex *directory_index);
uint32_t squash_format_inode_directory_index_start(
		const struct SquashInodeDirectoryIndex *directory_index);
uint32_t squash_format_inode_directory_index_name_size(
		const struct SquashInodeDirectoryIndex *directory_index);
const uint8_t *squash_format_inode_directory_index_name(
		const struct SquashInodeDirectoryIndex *directory_index);
#endif /* end of include guard FORMAT_INODE_H */
