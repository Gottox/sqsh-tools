/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         inode_data.c
 */

#include <cextras/endian.h>
#include <sqsh_data_private.h>

uint16_t
sqsh__data_inode_type(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU16(inode->header.type);
}
uint16_t
sqsh__data_inode_permissions(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU16(inode->header.permissions);
}
uint16_t
sqsh__data_inode_uid_idx(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU16(inode->header.uid_idx);
}
uint16_t
sqsh__data_inode_gid_idx(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU16(inode->header.gid_idx);
}
uint32_t
sqsh__data_inode_modified_time(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU32(inode->header.modified_time);
}
uint32_t
sqsh__data_inode_number(const struct SqshDataInode *inode) {
	return CX_LE_2_CPU32(inode->header.inode_number);
}

uint32_t
sqsh__data_inode_file_blocks_start(const struct SqshDataInodeFile *file) {
	return CX_LE_2_CPU32(file->blocks_start);
}
uint32_t
sqsh__data_inode_file_fragment_block_index(
		const struct SqshDataInodeFile *file) {
	return CX_LE_2_CPU32(file->fragment_block_index);
}
uint32_t
sqsh__data_inode_file_block_offset(const struct SqshDataInodeFile *file) {
	return CX_LE_2_CPU32(file->block_offset);
}
uint32_t
sqsh__data_inode_file_size(const struct SqshDataInodeFile *file) {
	return CX_LE_2_CPU32(file->file_size);
}
uint32_t
sqsh__data_inode_file_block_size_info(
		const struct SqshDataInodeFile *file, sqsh_index_t index) {
	const struct {
		uint32_t b;
	} SQSH_UNALIGNED *block_sizes = (const void *)&file[1];
	return CX_LE_2_CPU32(block_sizes[index].b);
}

uint64_t
sqsh__data_inode_file_ext_blocks_start(
		const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU64(file_ext->blocks_start);
}
uint64_t
sqsh__data_inode_file_ext_size(const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU64(file_ext->file_size);
}
uint64_t
sqsh__data_inode_file_ext_sparse(const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU64(file_ext->sparse);
}
uint32_t
sqsh__data_inode_file_ext_hard_link_count(
		const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU32(file_ext->hard_link_count);
}
uint32_t
sqsh__data_inode_file_ext_fragment_block_index(
		const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU32(file_ext->fragment_block_index);
}
uint32_t
sqsh__data_inode_file_ext_block_offset(
		const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU32(file_ext->block_offset);
}
uint32_t
sqsh__data_inode_file_ext_xattr_idx(
		const struct SqshDataInodeFileExt *file_ext) {
	return CX_LE_2_CPU32(file_ext->xattr_idx);
}
uint32_t
sqsh__data_inode_file_ext_block_size_info(
		const struct SqshDataInodeFileExt *file_ext, sqsh_index_t index) {
	const struct {
		uint32_t b;
	} SQSH_UNALIGNED *block_sizes = (const void *)&file_ext[1];
	return CX_LE_2_CPU32(block_sizes[index].b);
}

const struct SqshDataInodeDirectory *
sqsh__data_inode_directory(const struct SqshDataInode *inode) {
	return &inode->data.directory;
}
const struct SqshDataInodeDirectoryExt *
sqsh__data_inode_directory_ext(const struct SqshDataInode *inode) {
	return &inode->data.directory_ext;
}
const struct SqshDataInodeFile *
sqsh__data_inode_file(const struct SqshDataInode *inode) {
	return &inode->data.file;
}
const struct SqshDataInodeFileExt *
sqsh__data_inode_file_ext(const struct SqshDataInode *inode) {
	return &inode->data.file_ext;
}
const struct SqshDataInodeSymlink *
sqsh__data_inode_symlink(const struct SqshDataInode *inode) {
	return &inode->data.symlink;
}
const struct SqshDataInodeSymlinkExt *
sqsh__data_inode_symlink_ext(const struct SqshDataInode *inode) {
	return &inode->data.symlink_ext;
}
const struct SqshDataInodeDevice *
sqsh__data_inode_device(const struct SqshDataInode *inode) {
	return &inode->data.device;
}
const struct SqshDataInodeDeviceExt *
sqsh__data_inode_device_ext(const struct SqshDataInode *inode) {
	return &inode->data.device_ext;
}
const struct SqshDataInodeIpc *
sqsh__data_inode_ipc(const struct SqshDataInode *inode) {
	return &inode->data.ipc;
}
const struct SqshDataInodeIpcExt *
sqsh__data_inode_ipc_ext(const struct SqshDataInode *inode) {
	return &inode->data.ipc_ext;
}

uint32_t
sqsh__data_inode_directory_block_start(
		const struct SqshDataInodeDirectory *directory) {
	return CX_LE_2_CPU32(directory->block_start);
}
uint32_t
sqsh__data_inode_directory_hard_link_count(
		const struct SqshDataInodeDirectory *directory) {
	return CX_LE_2_CPU32(directory->hard_link_count);
}
uint16_t
sqsh__data_inode_directory_file_size(
		const struct SqshDataInodeDirectory *directory) {
	return CX_LE_2_CPU16(directory->file_size);
}
uint16_t
sqsh__data_inode_directory_block_offset(
		const struct SqshDataInodeDirectory *directory) {
	return CX_LE_2_CPU16(directory->block_offset);
}
uint32_t
sqsh__data_inode_directory_parent_inode_number(
		const struct SqshDataInodeDirectory *directory) {
	return CX_LE_2_CPU32(directory->parent_inode_number);
}

uint32_t
sqsh__data_inode_directory_ext_hard_link_count(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU32(directory_ext->hard_link_count);
}
uint32_t
sqsh__data_inode_directory_ext_file_size(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU32(directory_ext->file_size);
}
uint32_t
sqsh__data_inode_directory_ext_block_start(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU32(directory_ext->block_start);
}
uint32_t
sqsh__data_inode_directory_ext_parent_inode_number(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU32(directory_ext->parent_inode_number);
}
uint16_t
sqsh__data_inode_directory_ext_index_count(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU16(directory_ext->index_count);
}
uint16_t
sqsh__data_inode_directory_ext_block_offset(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU16(directory_ext->block_offset);
}
uint32_t
sqsh__data_inode_directory_ext_xattr_idx(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return CX_LE_2_CPU32(directory_ext->xattr_idx);
}
const uint8_t *
sqsh__data_inode_directory_ext_index(
		const struct SqshDataInodeDirectoryExt *directory_ext) {
	return (const uint8_t *)&directory_ext[1];
}

uint32_t
sqsh__data_inode_directory_index_index(
		const struct SqshDataInodeDirectoryIndex *directory_index) {
	return CX_LE_2_CPU32(directory_index->index);
}
uint32_t
sqsh__data_inode_directory_index_start(
		const struct SqshDataInodeDirectoryIndex *directory_index) {
	return CX_LE_2_CPU32(directory_index->start);
}
uint32_t
sqsh__data_inode_directory_index_name_size(
		const struct SqshDataInodeDirectoryIndex *directory_index) {
	return CX_LE_2_CPU32(directory_index->name_size);
}
const uint8_t *
sqsh__data_inode_directory_index_name(
		const struct SqshDataInodeDirectoryIndex *directory_index) {
	return (const uint8_t *)&directory_index[1];
}

uint32_t
sqsh__data_inode_symlink_hard_link_count(
		const struct SqshDataInodeSymlink *symlink) {
	return CX_LE_2_CPU32(symlink->hard_link_count);
}
uint32_t
sqsh__data_inode_symlink_target_size(
		const struct SqshDataInodeSymlink *symlink) {
	return CX_LE_2_CPU32(symlink->target_size);
}
const uint8_t *
sqsh__data_inode_symlink_target_path(
		const struct SqshDataInodeSymlink *symlink) {
	return (const uint8_t *)&symlink[1];
}

uint32_t
sqsh__data_inode_symlink_ext_hard_link_count(
		const struct SqshDataInodeSymlinkExt *symlink_ext) {
	return CX_LE_2_CPU32(symlink_ext->hard_link_count);
}
uint32_t
sqsh__data_inode_symlink_ext_target_size(
		const struct SqshDataInodeSymlinkExt *symlink_ext) {
	return CX_LE_2_CPU32(symlink_ext->target_size);
}
const uint8_t *
sqsh__data_inode_symlink_ext_target_path(
		const struct SqshDataInodeSymlinkExt *symlink_ext) {
	return (const uint8_t *)&symlink_ext[1];
}
uint32_t
sqsh__data_inode_symlink_ext_xattr_idx(
		const struct SqshDataInodeSymlinkExt *symlink_ext) {
	/*
	 * The xattr attributes of a symlink are located behind the target_path.
	 * What we do here is to fetch the size and the pointer of the symlink
	 * target and create a new uint32_t pointer behind the target path.
	 * So it looks like that:
	 *                          _
	 *  symlink_ext -----> [xxx] \
	 *                     [xxx]  |
	 *                     [xxx]  | sizeof(struct SqshDataInodeSymlinkExt)
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
			sqsh__data_inode_symlink_ext_target_size(symlink_ext) + 1;
	const uint8_t *target_path =
			sqsh__data_inode_symlink_ext_target_path(symlink_ext);
	const uint8_t *target_path_end = &target_path[target_size];
	const struct {
		uint32_t x;
	} SQSH_UNALIGNED *xattr_idx = (const void *)target_path_end;
	return CX_LE_2_CPU32(xattr_idx->x);
}

uint32_t
sqsh__data_inode_device_hard_link_count(
		const struct SqshDataInodeDevice *device) {
	return device->hard_link_count;
}
uint32_t
sqsh__data_inode_device_device(const struct SqshDataInodeDevice *device) {
	return device->device;
}

uint32_t
sqsh__data_inode_device_ext_hard_link_count(
		const struct SqshDataInodeDeviceExt *device) {
	return device->hard_link_count;
}
uint32_t
sqsh__data_inode_device_ext_device(
		const struct SqshDataInodeDeviceExt *device) {
	return device->device;
}
uint32_t
sqsh__data_inode_device_ext_xattr_idx(
		const struct SqshDataInodeDeviceExt *device) {
	return device->xattr_idx;
}

uint32_t
sqsh__data_inode_ipc_hard_link_count(const struct SqshDataInodeIpc *ipc) {
	return ipc->hard_link_count;
}

uint32_t
sqsh__data_inode_ipc_ext_hard_link_count(
		const struct SqshDataInodeIpcExt *ipc) {
	return ipc->hard_link_count;
}
uint32_t
sqsh__data_inode_ipc_ext_xattr_idx(const struct SqshDataInodeIpcExt *ipc) {
	return ipc->xattr_idx;
}
