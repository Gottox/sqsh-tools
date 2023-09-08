/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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

#define _DEFAULT_SOURCE

#include "../../../include/sqsh_data_set.h"

#include <cextras/endian_compat.h>

void
sqsh__data_inode_type_set(struct SqshDataInode *inode, const uint16_t value) {
	inode->header.type = htole16(value);
}
void
sqsh__data_inode_permissions_set(
		struct SqshDataInode *inode, const uint16_t value) {
	inode->header.permissions = htole16(value);
}
void
sqsh__data_inode_uid_idx_set(
		struct SqshDataInode *inode, const uint16_t value) {
	inode->header.uid_idx = htole16(value);
}
void
sqsh__data_inode_gid_idx_set(
		struct SqshDataInode *inode, const uint16_t value) {
	inode->header.gid_idx = htole16(value);
}
void
sqsh__data_inode_modified_time_set(
		struct SqshDataInode *inode, const uint32_t value) {
	inode->header.modified_time = htole32(value);
}
void
sqsh__data_inode_number_set(struct SqshDataInode *inode, const uint32_t value) {
	inode->header.inode_number = htole32(value);
}

struct SqshDataInodeDirectory *
sqsh__data_inode_directory_mut(struct SqshDataInode *inode) {
	return &inode->data.directory;
}
struct SqshDataInodeDirectoryExt *
sqsh__data_inode_directory_ext_mut(struct SqshDataInode *inode) {
	return &inode->data.directory_ext;
}
struct SqshDataInodeFile *
sqsh__data_inode_file_mut(struct SqshDataInode *inode) {
	return &inode->data.file;
}
struct SqshDataInodeFileExt *
sqsh__data_inode_file_ext_mut(struct SqshDataInode *inode) {
	return &inode->data.file_ext;
}
struct SqshDataInodeSymlink *
sqsh__data_inode_symlink_mut(struct SqshDataInode *inode) {
	return &inode->data.symlink;
}
struct SqshDataInodeSymlinkExt *
sqsh__data_inode_symlink_ext_mut(struct SqshDataInode *inode) {
	return &inode->data.symlink_ext;
}
struct SqshDataInodeDevice *
sqsh__data_inode_device_mut(struct SqshDataInode *inode) {
	return &inode->data.device;
}
struct SqshDataInodeDeviceExt *
sqsh__data_inode_device_ext_mut(struct SqshDataInode *inode) {
	return &inode->data.device_ext;
}
struct SqshDataInodeIpc *
sqsh__data_inode_ipc_mut(struct SqshDataInode *inode) {
	return &inode->data.ipc;
}
struct SqshDataInodeIpcExt *
sqsh__data_inode_ipc_ext_mut(struct SqshDataInode *inode) {
	return &inode->data.ipc_ext;
}

void
sqsh__data_inode_directory_index_index_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value) {
	directory_index->index = htole32(value);
}

void
sqsh__data_inode_directory_index_start_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value) {
	directory_index->start = htole32(value);
}

void
sqsh__data_inode_directory_index_name_size_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value) {
	directory_index->name_size = htole32(value);
}

void
sqsh__data_inode_directory_block_start_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value) {
	directory->block_start = htole32(value);
}

void
sqsh__data_inode_directory_hard_link_count_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value) {
	directory->hard_link_count = htole32(value);
}

void
sqsh__data_inode_directory_file_size_set(
		struct SqshDataInodeDirectory *directory, const uint16_t value) {
	directory->file_size = htole16(value);
}

void
sqsh__data_inode_directory_block_offset_set(
		struct SqshDataInodeDirectory *directory, const uint16_t value) {
	directory->block_offset = htole16(value);
}

void
sqsh__data_inode_directory_parent_inode_number_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value) {
	directory->parent_inode_number = htole32(value);
}

void
sqsh__data_inode_directory_ext_hard_link_count_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value) {
	directory_ext->hard_link_count = htole32(value);
}

void
sqsh__data_inode_directory_ext_file_size_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value) {
	directory_ext->file_size = htole32(value);
}

void
sqsh__data_inode_directory_ext_block_start_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value) {
	directory_ext->block_start = htole32(value);
}

void
sqsh__data_inode_directory_ext_parent_inode_number_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value) {
	directory_ext->parent_inode_number = htole32(value);
}

void
sqsh__data_inode_directory_ext_index_count_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint16_t value) {
	directory_ext->index_count = htole16(value);
}

void
sqsh__data_inode_directory_ext_block_offset_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint16_t value) {
	directory_ext->block_offset = htole16(value);
}

void
sqsh__data_inode_directory_ext_xattr_idx_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value) {
	directory_ext->xattr_idx = htole32(value);
}

void
sqsh__data_inode_symlink_ext_tail_xattr_idx_set(
		struct SqshDataInodeSymlinkExtTail *symlink_ext_tail,
		const uint32_t value) {
	symlink_ext_tail->xattr_idx = htole32(value);
}

void
sqsh__data_inode_symlink_target_size_set(
		struct SqshDataInodeSymlink *symlink, const uint32_t value) {
	symlink->target_size = htole32(value);
}

void
sqsh__data_inode_symlink_hard_link_count_set(
		struct SqshDataInodeSymlink *symlink, const uint32_t value) {
	symlink->hard_link_count = htole32(value);
}

void
sqsh__data_inode_symlink_ext_hard_link_count_set(
		struct SqshDataInodeSymlinkExt *symlink_ext, const uint32_t value) {
	symlink_ext->hard_link_count = htole32(value);
}

void
sqsh__data_inode_symlink_ext_target_size_set(
		struct SqshDataInodeSymlinkExt *symlink_ext, const uint32_t value) {
	symlink_ext->target_size = htole32(value);
}
