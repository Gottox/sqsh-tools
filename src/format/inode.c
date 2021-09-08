/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Wednesday Sep 08, 2021 13:02:43 CEST
 */

#include "inode_internal.h"
#include <endian.h>

uint16_t
squash_format_inode_type(const struct SquashInode *inode) {
	return htole16(inode->header.type);
}
uint16_t
squash_format_inode_permissions(const struct SquashInode *inode) {
	return htole16(inode->header.permissions);
}
uint16_t
squash_format_inode_uid_idx(const struct SquashInode *inode) {
	return htole16(inode->header.uid_idx);
}
uint16_t
squash_format_inode_gid_idx(const struct SquashInode *inode) {
	return htole16(inode->header.gid_idx);
}
uint32_t
squash_format_inode_modified_time(const struct SquashInode *inode) {
	return htole16(inode->header.modified_time);
}
uint32_t
squash_format_inode_inode_number(const struct SquashInode *inode) {
	return htole16(inode->header.inode_number);
}

const struct SquashInodeDirectory *
squash_format_inode_directory(const struct SquashInode *inode) {
	return &inode->data.directory;
}
const struct SquashInodeDirectoryExt *
squash_format_inode_directory_ext(const struct SquashInode *inode) {
	return &inode->data.directory_ext;
}
const struct SquashInodeFile *
squash_format_inode_file(const struct SquashInode *inode) {
	return &inode->data.file;
}
const struct SquashInodeFileExt *
squash_format_inode_file_ext(const struct SquashInode *inode) {
	return &inode->data.file_ext;
}
const struct SquashInodeSymlink *
squash_format_inode_symlink(const struct SquashInode *inode) {
	return &inode->data.symlink;
}
const struct SquashInodeSymlinkExt *
squash_format_inode_symlink_ext(const struct SquashInode *inode) {
	return &inode->data.symlink_ext;
}
const struct SquashInodeDevice *
squash_format_inode_device(const struct SquashInode *inode) {
	return &inode->data.device;
}
const struct SquashInodeDeviceExt *
squash_format_inode_device_ext(const struct SquashInode *inode) {
	return &inode->data.device_ext;
}
const struct SquashInodeIpc *
squash_format_inode_ipc(const struct SquashInode *inode) {
	return &inode->data.ipc;
}
const struct SquashInodeIpcExt *
squash_format_inode_ipc_ext(const struct SquashInode *inode) {
	return &inode->data.ipc_ext;
}

uint32_t
squash_format_inode_directory_block_start(
		const struct SquashInodeDirectory *directory) {
	return directory->block_start;
}
uint32_t
squash_format_inode_directory_hard_link_count(
		const struct SquashInodeDirectory *directory) {
	return directory->hard_link_count;
}
uint16_t
squash_format_inode_directory_file_size(
		const struct SquashInodeDirectory *directory) {
	return directory->file_size;
}
uint16_t
squash_format_inode_directory_block_offset(
		const struct SquashInodeDirectory *directory) {
	return directory->block_offset;
}
uint32_t
squash_format_inode_directory_parent_inode_number(
		const struct SquashInodeDirectory *directory) {
	return directory->parent_inode_number;
}

uint32_t
squash_format_inode_directory_ext_hard_link_count(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->hard_link_count;
}
uint32_t
squash_format_inode_directory_ext_file_size(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->file_size;
}
uint32_t
squash_format_inode_directory_ext_dir_block_start(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->block_start;
}
uint32_t
squash_format_inode_directory_ext_parent_inode_number(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->index_count;
}
uint16_t
squash_format_inode_directory_ext_index_count(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->index_count;
}
uint16_t
squash_format_inode_directory_ext_block_offset(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->block_offset;
}
uint32_t
squash_format_inode_directory_ext_xattr_idx(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return directory_ext->xattr_idx;
}
const struct SquashInodeDirectoryIndex *
squash_format_inode_directory_ext_index(
		const struct SquashInodeDirectoryExt *directory_ext) {
	const uint8_t *tmp = (const uint8_t *)directory_ext;
	return (const struct SquashInodeDirectoryIndex
					*)&tmp[sizeof(struct SquashInodeDirectoryExt)];
}

uint32_t
squash_format_inode_directory_index_index(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->index);
}
uint32_t
squash_format_inode_directory_index_start(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->start);
}
uint32_t
squash_format_inode_directory_index_name_size(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->name_size);
}
const uint8_t *
squash_format_inode_directory_index_name(
		const struct SquashInodeDirectoryIndex *directory_index) {
	const uint8_t *tmp = (const uint8_t *)directory_index;
	return &tmp[sizeof(struct SquashInodeDirectoryIndex)];
}
