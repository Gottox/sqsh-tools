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
hsqs_data_inode_type(const struct HsqsInode *inode) {
	return htole16(inode->header.type);
}
uint16_t
hsqs_data_inode_permissions(const struct HsqsInode *inode) {
	return htole16(inode->header.permissions);
}
uint16_t
hsqs_data_inode_uid_idx(const struct HsqsInode *inode) {
	return htole16(inode->header.uid_idx);
}
uint16_t
hsqs_data_inode_gid_idx(const struct HsqsInode *inode) {
	return htole16(inode->header.gid_idx);
}
uint32_t
hsqs_data_inode_modified_time(const struct HsqsInode *inode) {
	return htole32(inode->header.modified_time);
}
uint32_t
hsqs_data_inode_number(const struct HsqsInode *inode) {
	return htole32(inode->header.inode_number);
}

uint32_t
hsqs_data_inode_file_blocks_start(const struct HsqsInodeFile *file) {
	return htole32(file->blocks_start);
}
uint32_t
hsqs_data_inode_file_fragment_block_index(const struct HsqsInodeFile *file) {
	return htole32(file->fragment_block_index);
}
uint32_t
hsqs_data_inode_file_block_offset(const struct HsqsInodeFile *file) {
	return htole32(file->block_offset);
}
uint32_t
hsqs_data_inode_file_size(const struct HsqsInodeFile *file) {
	return htole32(file->file_size);
}
const struct HsqsDatablockSize *
hsqs_data_inode_file_block_sizes(const struct HsqsInodeFile *file) {
	return (const struct HsqsDatablockSize *)&file[1];
}

uint64_t
hsqs_data_inode_file_ext_blocks_start(const struct HsqsInodeFileExt *file_ext) {
	return htole64(file_ext->blocks_start);
}
uint64_t
hsqs_data_inode_file_ext_size(const struct HsqsInodeFileExt *file_ext) {
	return htole64(file_ext->file_size);
}
uint64_t
hsqs_data_inode_file_ext_sparse(const struct HsqsInodeFileExt *file_ext) {
	return htole64(file_ext->sparse);
}
uint32_t
hsqs_data_inode_file_ext_hard_link_count(
		const struct HsqsInodeFileExt *file_ext) {
	return htole32(file_ext->hard_link_count);
}
uint32_t
hsqs_data_inode_file_ext_fragment_block_index(
		const struct HsqsInodeFileExt *file_ext) {
	return htole32(file_ext->fragment_block_index);
}
uint32_t
hsqs_data_inode_file_ext_block_offset(const struct HsqsInodeFileExt *file_ext) {
	return htole32(file_ext->block_offset);
}
uint32_t
hsqs_data_inode_file_ext_xattr_idx(const struct HsqsInodeFileExt *file_ext) {
	return htole32(file_ext->xattr_idx);
}
const struct HsqsDatablockSize *
hsqs_data_inode_file_ext_block_sizes(const struct HsqsInodeFileExt *file_ext) {
	return (const struct HsqsDatablockSize *)&file_ext[1];
}

const struct HsqsInodeDirectory *
hsqs_data_inode_directory(const struct HsqsInode *inode) {
	return &inode->data.directory;
}
const struct HsqsInodeDirectoryExt *
hsqs_data_inode_directory_ext(const struct HsqsInode *inode) {
	return &inode->data.directory_ext;
}
const struct HsqsInodeFile *
hsqs_data_inode_file(const struct HsqsInode *inode) {
	return &inode->data.file;
}
const struct HsqsInodeFileExt *
hsqs_data_inode_file_ext(const struct HsqsInode *inode) {
	return &inode->data.file_ext;
}
const struct HsqsInodeSymlink *
hsqs_data_inode_symlink(const struct HsqsInode *inode) {
	return &inode->data.symlink;
}
const struct HsqsInodeSymlinkExt *
hsqs_data_inode_symlink_ext(const struct HsqsInode *inode) {
	return &inode->data.symlink_ext;
}
const struct HsqsInodeDevice *
hsqs_data_inode_device(const struct HsqsInode *inode) {
	return &inode->data.device;
}
const struct HsqsInodeDeviceExt *
hsqs_data_inode_device_ext(const struct HsqsInode *inode) {
	return &inode->data.device_ext;
}
const struct HsqsInodeIpc *
hsqs_data_inode_ipc(const struct HsqsInode *inode) {
	return &inode->data.ipc;
}
const struct HsqsInodeIpcExt *
hsqs_data_inode_ipc_ext(const struct HsqsInode *inode) {
	return &inode->data.ipc_ext;
}

uint32_t
hsqs_data_inode_directory_block_start(
		const struct HsqsInodeDirectory *directory) {
	return htole32(directory->block_start);
}
uint32_t
hsqs_data_inode_directory_hard_link_count(
		const struct HsqsInodeDirectory *directory) {
	return htole32(directory->hard_link_count);
}
uint16_t
hsqs_data_inode_directory_file_size(
		const struct HsqsInodeDirectory *directory) {
	return htole16(directory->file_size);
}
uint16_t
hsqs_data_inode_directory_block_offset(
		const struct HsqsInodeDirectory *directory) {
	return htole16(directory->block_offset);
}
uint32_t
hsqs_data_inode_directory_parent_inode_number(
		const struct HsqsInodeDirectory *directory) {
	return htole32(directory->parent_inode_number);
}

uint32_t
hsqs_data_inode_directory_ext_hard_link_count(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->hard_link_count);
}
uint32_t
hsqs_data_inode_directory_ext_file_size(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->file_size);
}
uint32_t
hsqs_data_inode_directory_ext_block_start(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->block_start);
}
uint32_t
hsqs_data_inode_directory_ext_parent_inode_number(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->parent_inode_number);
}
uint16_t
hsqs_data_inode_directory_ext_index_count(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->index_count);
}
uint16_t
hsqs_data_inode_directory_ext_block_offset(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->block_offset);
}
uint32_t
hsqs_data_inode_directory_ext_xattr_idx(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->xattr_idx);
}
const struct HsqsInodeDirectoryIndex *
hsqs_data_inode_directory_ext_index(
		const struct HsqsInodeDirectoryExt *directory_ext) {
	return (const struct HsqsInodeDirectoryIndex *)&directory_ext[1];
}

uint32_t
hsqs_data_inode_directory_index_index(
		const struct HsqsInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->index);
}
uint32_t
hsqs_data_inode_directory_index_start(
		const struct HsqsInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->start);
}
uint32_t
hsqs_data_inode_directory_index_name_size(
		const struct HsqsInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->name_size);
}
const uint8_t *
hsqs_data_inode_directory_index_name(
		const struct HsqsInodeDirectoryIndex *directory_index) {
	return (const uint8_t *)&directory_index[1];
}

uint32_t
hsqs_data_inode_symlink_hard_link_count(
		const struct HsqsInodeSymlink *symlink) {
	return le32toh(symlink->hard_link_count);
}
uint32_t
hsqs_data_inode_symlink_target_size(const struct HsqsInodeSymlink *symlink) {
	return le32toh(symlink->target_size);
}
const uint8_t *
hsqs_data_inode_symlink_target_path(const struct HsqsInodeSymlink *symlink) {
	return (const uint8_t *)&symlink[1];
}

uint32_t
hsqs_data_inode_symlink_ext_hard_link_count(
		const struct HsqsInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->hard_link_count);
}
uint32_t
hsqs_data_inode_symlink_ext_target_size(
		const struct HsqsInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->target_size);
}
const uint8_t *
hsqs_data_inode_symlink_ext_target_path(
		const struct HsqsInodeSymlinkExt *symlink_ext) {
	return (const uint8_t *)&symlink_ext[1];
}
uint32_t
hsqs_data_inode_symlink_ext_xattr_idx(
		const struct HsqsInodeSymlinkExt *symlink_ext) {
	/*
	 * The xattr attributes of a symlink are located behind the target_path.
	 * What we do here is to fetch the size and the pointer of the symlink
	 * target and create a new uint32_t pointer behind the target path.
	 * So it looks like that:
	 *                          _
	 *  symlink_ext -----> [xxx] \
	 *                     [xxx]  |
	 *                     [xxx]  | sizeof(struct HsqsInodeSymlinkExt)
	 *                     [xxx]  |
	 *                     [xxx]_/
	 *  target_path -----> [ 0 ] \
	 *                     [ 1 ]  |
	 *                     [ 2 ]  | target_size
	 *                     [...]  |
	 *                     [ n ]_/
	 *    xattr_idx -----> [n+1]
	 */
	const uint32_t target_size =
			hsqs_data_inode_symlink_ext_target_size(symlink_ext) + 1;
	const uint8_t *target_path =
			hsqs_data_inode_symlink_ext_target_path(symlink_ext);
	const uint8_t *target_path_end = &target_path[target_size];
	const uint32_t *xattr_idx = (const uint32_t *)target_path_end;
	return le32toh(*xattr_idx);
}

uint32_t
hsqs_data_inode_device_hard_link_count(const struct HsqsInodeDevice *device) {
	return device->hard_link_count;
}
uint32_t
hsqs_data_inode_device_device(const struct HsqsInodeDevice *device) {
	return device->device;
}

uint32_t
hsqs_data_inode_device_ext_hard_link_count(
		const struct HsqsInodeDeviceExt *device) {
	return device->hard_link_count;
}
uint32_t
hsqs_data_inode_device_ext_device(const struct HsqsInodeDeviceExt *device) {
	return device->device;
}
uint32_t
hsqs_data_inode_device_ext_xattr_idx(const struct HsqsInodeDeviceExt *device) {
	return device->xattr_idx;
}

uint32_t
hsqs_data_inode_ipc_hard_link_count(const struct HsqsInodeIpc *ipc) {
	return ipc->hard_link_count;
}

uint32_t
hsqs_data_inode_ipc_ext_hard_link_count(const struct HsqsInodeIpcExt *ipc) {
	return ipc->hard_link_count;
}
uint32_t
hsqs_data_inode_ipc_ext_xattr_idx(const struct HsqsInodeIpcExt *ipc) {
	return ipc->xattr_idx;
}
