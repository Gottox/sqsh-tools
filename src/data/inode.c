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
 * @file         inode.c
 */

#include "inode_internal.h"
#include <endian.h>
#include <stdint.h>

uint16_t
sqsh_data_inode_type(const struct SqshInode *inode) {
	return htole16(inode->header.type);
}
uint16_t
sqsh_data_inode_permissions(const struct SqshInode *inode) {
	return htole16(inode->header.permissions);
}
uint16_t
sqsh_data_inode_uid_idx(const struct SqshInode *inode) {
	return htole16(inode->header.uid_idx);
}
uint16_t
sqsh_data_inode_gid_idx(const struct SqshInode *inode) {
	return htole16(inode->header.gid_idx);
}
uint32_t
sqsh_data_inode_modified_time(const struct SqshInode *inode) {
	return htole32(inode->header.modified_time);
}
uint32_t
sqsh_data_inode_number(const struct SqshInode *inode) {
	return htole32(inode->header.inode_number);
}

uint32_t
sqsh_data_inode_file_blocks_start(const struct SqshInodeFile *file) {
	return htole32(file->blocks_start);
}
uint32_t
sqsh_data_inode_file_fragment_block_index(const struct SqshInodeFile *file) {
	return htole32(file->fragment_block_index);
}
uint32_t
sqsh_data_inode_file_block_offset(const struct SqshInodeFile *file) {
	return htole32(file->block_offset);
}
uint32_t
sqsh_data_inode_file_size(const struct SqshInodeFile *file) {
	return htole32(file->file_size);
}
const struct SqshDatablockSize *
sqsh_data_inode_file_block_sizes(const struct SqshInodeFile *file) {
	return (const struct SqshDatablockSize *)&file[1];
}

uint64_t
sqsh_data_inode_file_ext_blocks_start(const struct SqshInodeFileExt *file_ext) {
	return htole64(file_ext->blocks_start);
}
uint64_t
sqsh_data_inode_file_ext_size(const struct SqshInodeFileExt *file_ext) {
	return htole64(file_ext->file_size);
}
uint64_t
sqsh_data_inode_file_ext_sparse(const struct SqshInodeFileExt *file_ext) {
	return htole64(file_ext->sparse);
}
uint32_t
sqsh_data_inode_file_ext_hard_link_count(
		const struct SqshInodeFileExt *file_ext) {
	return htole32(file_ext->hard_link_count);
}
uint32_t
sqsh_data_inode_file_ext_fragment_block_index(
		const struct SqshInodeFileExt *file_ext) {
	return htole32(file_ext->fragment_block_index);
}
uint32_t
sqsh_data_inode_file_ext_block_offset(const struct SqshInodeFileExt *file_ext) {
	return htole32(file_ext->block_offset);
}
uint32_t
sqsh_data_inode_file_ext_xattr_idx(const struct SqshInodeFileExt *file_ext) {
	return htole32(file_ext->xattr_idx);
}
const struct SqshDatablockSize *
sqsh_data_inode_file_ext_block_sizes(const struct SqshInodeFileExt *file_ext) {
	return (const struct SqshDatablockSize *)&file_ext[1];
}

const struct SqshInodeDirectory *
sqsh_data_inode_directory(const struct SqshInode *inode) {
	return &inode->data.directory;
}
const struct SqshInodeDirectoryExt *
sqsh_data_inode_directory_ext(const struct SqshInode *inode) {
	return &inode->data.directory_ext;
}
const struct SqshInodeFile *
sqsh_data_inode_file(const struct SqshInode *inode) {
	return &inode->data.file;
}
const struct SqshInodeFileExt *
sqsh_data_inode_file_ext(const struct SqshInode *inode) {
	return &inode->data.file_ext;
}
const struct SqshInodeSymlink *
sqsh_data_inode_symlink(const struct SqshInode *inode) {
	return &inode->data.symlink;
}
const struct SqshInodeSymlinkExt *
sqsh_data_inode_symlink_ext(const struct SqshInode *inode) {
	return &inode->data.symlink_ext;
}
const struct SqshInodeDevice *
sqsh_data_inode_device(const struct SqshInode *inode) {
	return &inode->data.device;
}
const struct SqshInodeDeviceExt *
sqsh_data_inode_device_ext(const struct SqshInode *inode) {
	return &inode->data.device_ext;
}
const struct SqshInodeIpc *
sqsh_data_inode_ipc(const struct SqshInode *inode) {
	return &inode->data.ipc;
}
const struct SqshInodeIpcExt *
sqsh_data_inode_ipc_ext(const struct SqshInode *inode) {
	return &inode->data.ipc_ext;
}

uint32_t
sqsh_data_inode_directory_block_start(
		const struct SqshInodeDirectory *directory) {
	return htole32(directory->block_start);
}
uint32_t
sqsh_data_inode_directory_hard_link_count(
		const struct SqshInodeDirectory *directory) {
	return htole32(directory->hard_link_count);
}
uint16_t
sqsh_data_inode_directory_file_size(
		const struct SqshInodeDirectory *directory) {
	return htole16(directory->file_size);
}
uint16_t
sqsh_data_inode_directory_block_offset(
		const struct SqshInodeDirectory *directory) {
	return htole16(directory->block_offset);
}
uint32_t
sqsh_data_inode_directory_parent_inode_number(
		const struct SqshInodeDirectory *directory) {
	return htole32(directory->parent_inode_number);
}

uint32_t
sqsh_data_inode_directory_ext_hard_link_count(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->hard_link_count);
}
uint32_t
sqsh_data_inode_directory_ext_file_size(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->file_size);
}
uint32_t
sqsh_data_inode_directory_ext_block_start(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->block_start);
}
uint32_t
sqsh_data_inode_directory_ext_parent_inode_number(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->parent_inode_number);
}
uint16_t
sqsh_data_inode_directory_ext_index_count(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->index_count);
}
uint16_t
sqsh_data_inode_directory_ext_block_offset(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole16(directory_ext->block_offset);
}
uint32_t
sqsh_data_inode_directory_ext_xattr_idx(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return htole32(directory_ext->xattr_idx);
}
const uint8_t *
sqsh_data_inode_directory_ext_index(
		const struct SqshInodeDirectoryExt *directory_ext) {
	return (const uint8_t *)&directory_ext[1];
}

uint32_t
sqsh_data_inode_directory_index_index(
		const struct SqshInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->index);
}
uint32_t
sqsh_data_inode_directory_index_start(
		const struct SqshInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->start);
}
uint32_t
sqsh_data_inode_directory_index_name_size(
		const struct SqshInodeDirectoryIndex *directory_index) {
	return le32toh(directory_index->name_size);
}
const uint8_t *
sqsh_data_inode_directory_index_name(
		const struct SqshInodeDirectoryIndex *directory_index) {
	return (const uint8_t *)&directory_index[1];
}

uint32_t
sqsh_data_inode_symlink_hard_link_count(
		const struct SqshInodeSymlink *symlink) {
	return le32toh(symlink->hard_link_count);
}
uint32_t
sqsh_data_inode_symlink_target_size(const struct SqshInodeSymlink *symlink) {
	return le32toh(symlink->target_size);
}
const uint8_t *
sqsh_data_inode_symlink_target_path(const struct SqshInodeSymlink *symlink) {
	return (const uint8_t *)&symlink[1];
}

uint32_t
sqsh_data_inode_symlink_ext_hard_link_count(
		const struct SqshInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->hard_link_count);
}
uint32_t
sqsh_data_inode_symlink_ext_target_size(
		const struct SqshInodeSymlinkExt *symlink_ext) {
	return le32toh(symlink_ext->target_size);
}
const uint8_t *
sqsh_data_inode_symlink_ext_target_path(
		const struct SqshInodeSymlinkExt *symlink_ext) {
	return (const uint8_t *)&symlink_ext[1];
}
uint32_t
sqsh_data_inode_symlink_ext_xattr_idx(
		const struct SqshInodeSymlinkExt *symlink_ext) {
	/*
	 * The xattr attributes of a symlink are located behind the target_path.
	 * What we do here is to fetch the size and the pointer of the symlink
	 * target and create a new uint32_t pointer behind the target path.
	 * So it looks like that:
	 *                          _
	 *  symlink_ext -----> [xxx] \
	 *                     [xxx]  |
	 *                     [xxx]  | sizeof(struct SqshInodeSymlinkExt)
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
			sqsh_data_inode_symlink_ext_target_size(symlink_ext) + 1;
	const uint8_t *target_path =
			sqsh_data_inode_symlink_ext_target_path(symlink_ext);
	const uint8_t *target_path_end = &target_path[target_size];
	const uint32_t *xattr_idx = (const uint32_t *)target_path_end;
	return le32toh(*xattr_idx);
}

uint32_t
sqsh_data_inode_device_hard_link_count(const struct SqshInodeDevice *device) {
	return device->hard_link_count;
}
uint32_t
sqsh_data_inode_device_device(const struct SqshInodeDevice *device) {
	return device->device;
}

uint32_t
sqsh_data_inode_device_ext_hard_link_count(
		const struct SqshInodeDeviceExt *device) {
	return device->hard_link_count;
}
uint32_t
sqsh_data_inode_device_ext_device(const struct SqshInodeDeviceExt *device) {
	return device->device;
}
uint32_t
sqsh_data_inode_device_ext_xattr_idx(const struct SqshInodeDeviceExt *device) {
	return device->xattr_idx;
}

uint32_t
sqsh_data_inode_ipc_hard_link_count(const struct SqshInodeIpc *ipc) {
	return ipc->hard_link_count;
}

uint32_t
sqsh_data_inode_ipc_ext_hard_link_count(const struct SqshInodeIpcExt *ipc) {
	return ipc->hard_link_count;
}
uint32_t
sqsh_data_inode_ipc_ext_xattr_idx(const struct SqshInodeIpcExt *ipc) {
	return ipc->xattr_idx;
}
