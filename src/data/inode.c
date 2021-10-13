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
 * @created     : Wednesday Sep 08, 2021 13:02:43 CEST
 */

#include "inode_internal.h"
#include <endian.h>
#include <stdint.h>

uint16_t
squash_data_inode_type(const struct SquashInode *inode) {
	return htole16(inode->header.type);
}
uint16_t
squash_data_inode_permissions(const struct SquashInode *inode) {
	return htole16(inode->header.permissions);
}
uint16_t
squash_data_inode_uid_idx(const struct SquashInode *inode) {
	return htole16(inode->header.uid_idx);
}
uint16_t
squash_data_inode_gid_idx(const struct SquashInode *inode) {
	return htole16(inode->header.gid_idx);
}
uint32_t
squash_data_inode_modified_time(const struct SquashInode *inode) {
	return htole32(inode->header.modified_time);
}
uint32_t
squash_data_inode_number(const struct SquashInode *inode) {
	return htole32(inode->header.inode_number);
}

uint32_t
squash_data_inode_file_blocks_start(const struct SquashInodeFile *file) {
	return htole32(file->blocks_start);
}
uint32_t
squash_data_inode_file_fragment_block_index(
		const struct SquashInodeFile *file) {
	return htole32(file->fragment_block_index);
}
uint32_t
squash_data_inode_file_block_offset(const struct SquashInodeFile *file) {
	return htole32(file->block_offset);
}
uint32_t
squash_data_inode_file_size(const struct SquashInodeFile *file) {
	return htole32(file->file_size);
}
const struct SquashDatablockSize *
squash_data_inode_file_block_sizes(const struct SquashInodeFile *file) {
	const uint8_t *tmp = (const uint8_t *)file;
	return (const struct SquashDatablockSize *)&tmp[SQUASH_SIZEOF_INODE_FILE];
}

uint64_t
squash_data_inode_file_ext_blocks_start(
		const struct SquashInodeFileExt *file_ext) {
	return htole64(file_ext->blocks_start);
}
uint64_t
squash_data_inode_file_ext_size(const struct SquashInodeFileExt *file_ext) {
	return htole64(file_ext->file_size);
}
uint64_t
squash_data_inode_file_ext_sparse(const struct SquashInodeFileExt *file_ext) {
	return htole64(file_ext->sparse);
}
uint32_t
squash_data_inode_file_ext_hard_link_count(
		const struct SquashInodeFileExt *file_ext) {
	return htole32(file_ext->hard_link_count);
}
uint32_t
squash_data_inode_file_ext_fragment_block_index(
		const struct SquashInodeFileExt *file_ext) {
	return htole32(file_ext->fragment_block_index);
}
uint32_t
squash_data_inode_file_ext_block_offset(
		const struct SquashInodeFileExt *file_ext) {
	return htole32(file_ext->block_offset);
}
uint32_t
squash_data_inode_file_ext_xattr_idx(
		const struct SquashInodeFileExt *file_ext) {
	return htole32(file_ext->xattr_idx);
}
const struct SquashDatablockSize *
squash_data_inode_file_ext_block_sizes(
		const struct SquashInodeFileExt *file_ext) {
	const uint8_t *tmp = (const uint8_t *)file_ext;
	return (const struct SquashDatablockSize
					*)&tmp[sizeof(struct SquashInodeFile)];
}

const struct SquashInodeDirectory *
squash_data_inode_directory(const struct SquashInode *inode) {
	return &inode->data.directory;
}
const struct SquashInodeDirectoryExt *
squash_data_inode_directory_ext(const struct SquashInode *inode) {
	return &inode->data.directory_ext;
}
const struct SquashInodeFile *
squash_data_inode_file(const struct SquashInode *inode) {
	return &inode->data.file;
}
const struct SquashInodeFileExt *
squash_data_inode_file_ext(const struct SquashInode *inode) {
	return &inode->data.file_ext;
}
const struct SquashInodeSymlink *
squash_data_inode_symlink(const struct SquashInode *inode) {
	return &inode->data.symlink;
}
const struct SquashInodeSymlinkExt *
squash_data_inode_symlink_ext(const struct SquashInode *inode) {
	return &inode->data.symlink_ext;
}
const struct SquashInodeDevice *
squash_data_inode_device(const struct SquashInode *inode) {
	return &inode->data.device;
}
const struct SquashInodeDeviceExt *
squash_data_inode_device_ext(const struct SquashInode *inode) {
	return &inode->data.device_ext;
}
const struct SquashInodeIpc *
squash_data_inode_ipc(const struct SquashInode *inode) {
	return &inode->data.ipc;
}
const struct SquashInodeIpcExt *
squash_data_inode_ipc_ext(const struct SquashInode *inode) {
	return &inode->data.ipc_ext;
}

uint32_t
squash_data_inode_directory_block_start(
		const struct SquashInodeDirectory *directory) {
	return htole32(directory->block_start);
}
uint32_t
squash_data_inode_directory_hard_link_count(
		const struct SquashInodeDirectory *directory) {
	return htole32(directory->hard_link_count);
}
uint16_t
squash_data_inode_directory_file_size(
		const struct SquashInodeDirectory *directory) {
	return htole16(directory->file_size);
}
uint16_t
squash_data_inode_directory_block_offset(
		const struct SquashInodeDirectory *directory) {
	return htole16(directory->block_offset);
}
uint32_t
squash_data_inode_directory_parent_inode_number(
		const struct SquashInodeDirectory *directory) {
	return htole32(directory->parent_inode_number);
}

uint32_t
squash_data_inode_directory_ext_hard_link_count(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->hard_link_count);
}
uint32_t
squash_data_inode_directory_ext_file_size(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->file_size);
}
uint32_t
squash_data_inode_directory_ext_block_start(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->block_start);
}
uint32_t
squash_data_inode_directory_ext_parent_inode_number(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->index_count);
}
uint16_t
squash_data_inode_directory_ext_index_count(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->index_count);
}
uint16_t
squash_data_inode_directory_ext_block_offset(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->block_offset);
}
uint32_t
squash_data_inode_directory_ext_xattr_idx(
		const struct SquashInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->xattr_idx);
}
const struct SquashInodeDirectoryIndex *
squash_data_inode_directory_ext_index(
		const struct SquashInodeDirectoryExt *directory_ext) {
	const uint8_t *tmp = (const uint8_t *)directory_ext;
	return (const struct SquashInodeDirectoryIndex
					*)&tmp[sizeof(struct SquashInodeDirectoryExt)];
}

uint32_t
squash_data_inode_directory_index_index(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->index);
}
uint32_t
squash_data_inode_directory_index_start(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->start);
}
uint32_t
squash_data_inode_directory_index_name_size(
		const struct SquashInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->name_size);
}
const uint8_t *
squash_data_inode_directory_index_name(
		const struct SquashInodeDirectoryIndex *directory_index) {
	const uint8_t *tmp = (const uint8_t *)directory_index;
	return &tmp[sizeof(struct SquashInodeDirectoryIndex)];
}

uint32_t
squash_data_inode_symlink_hard_link_count(
		const struct SquashInodeSymlink *symlink) {
	return le32toh(symlink->hard_link_count);
}
uint32_t
squash_data_inode_symlink_target_size(
		const struct SquashInodeSymlink *symlink) {
	return le32toh(symlink->target_size);
}
const uint8_t *
squash_data_inode_symlink_target_path(
		const struct SquashInodeSymlink *symlink) {
	const uint8_t *tmp = (const uint8_t *)symlink;
	return &tmp[sizeof(struct SquashInodeSymlink)];
}

uint32_t
squash_data_inode_symlink_ext_hard_link_count(
		const struct SquashInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->hard_link_count);
}
uint32_t
squash_data_inode_symlink_ext_target_size(
		const struct SquashInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->target_size);
}
const uint8_t *
squash_data_inode_symlink_ext_target_path(
		const struct SquashInodeSymlinkExt *symlink_ext) {
	const uint8_t *tmp = (const uint8_t *)symlink_ext;
	return &tmp[sizeof(struct SquashInodeSymlinkExt)];
}
uint32_t
squash_data_inode_symlink_ext_xattr_idx(
		const struct SquashInodeSymlinkExt *symlink_ext) {
	const uint32_t target_size =
			squash_data_inode_symlink_ext_target_size(symlink_ext) + 1;
	const uint8_t *tmp = (const uint8_t *)symlink_ext;
	tmp = &tmp[sizeof(struct SquashInodeSymlinkExt) + target_size];
	return le32toh(*((uint32_t *)tmp));
}

uint32_t
squash_data_inode_device_hard_link_count(
		const struct SquashInodeDevice *device) {
	return device->hard_link_count;
}
uint32_t
squash_data_inode_device_device(const struct SquashInodeDevice *device) {
	return device->device;
}

uint32_t
squash_data_inode_device_ext_hard_link_count(
		const struct SquashInodeDeviceExt *device) {
	return device->hard_link_count;
}
uint32_t
squash_data_inode_device_ext_device(const struct SquashInodeDeviceExt *device) {
	return device->device;
}
uint32_t
squash_data_inode_device_ext_xattr_idx(
		const struct SquashInodeDeviceExt *device) {
	return device->xattr_idx;
}

uint32_t
squash_data_inode_ipc_hard_link_count(const struct SquashInodeIpc *ipc) {
	return ipc->hard_link_count;
}

uint32_t
squash_data_inode_ipc_ext_hard_link_count(const struct SquashInodeIpcExt *ipc) {
	return ipc->hard_link_count;
}
uint32_t
squash_data_inode_ipc_ext_xattr_idx(const struct SquashInodeIpcExt *ipc) {
	return ipc->xattr_idx;
}
