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

/*
 * @author       Enno Boland (mail@eboland.de)
 * @file         inode.h
 */

#include "datablock.h"
#include <stdint.h>

#ifndef INODE_H

#define INODE_H

#define HSQS_SIZEOF_INODE_DIRECTORY_INDEX 12
#define HSQS_SIZEOF_INODE_DIRECTORY 16
#define HSQS_SIZEOF_INODE_DIRECTORY_EXT 24
#define HSQS_SIZEOF_INODE_FILE 16
#define HSQS_SIZEOF_INODE_FILE_EXT 40
#define HSQS_SIZEOF_INODE_SYMLINK 8
#define HSQS_SIZEOF_INODE_SYMLINK_EXT 8
#define HSQS_SIZEOF_INODE_SYMLINK_EXT_TAIL 4
#define HSQS_SIZEOF_INODE_DEVICE 8
#define HSQS_SIZEOF_INODE_DEVICE_EXT 12
#define HSQS_SIZEOF_INODE_IPC 4
#define HSQS_SIZEOF_INODE_IPC_EXT 8
#define HSQS_SIZEOF_INODE_HEADER 16

struct SqshInodeDirectory;
struct SqshInodeDirectoryExt;
struct SqshInodeFile;
struct SqshInodeFileExt;
struct SqshInodeSymlink;
struct SqshInodeSymlinkExt;
struct SqshInodeSymlinkExtTail;
struct SqshInodeDevice;
struct SqshInodeDeviceExt;
struct SqshInodeIpc;
struct SqshInodeIpcExt;
struct SqshInode;

struct SqshInodeDirectoryIndex;

uint16_t sqsh_data_inode_type(const struct SqshInode *inode);
uint16_t sqsh_data_inode_permissions(const struct SqshInode *inode);
uint16_t sqsh_data_inode_uid_idx(const struct SqshInode *inode);
uint16_t sqsh_data_inode_gid_idx(const struct SqshInode *inode);
uint32_t sqsh_data_inode_modified_time(const struct SqshInode *inode);
uint32_t sqsh_data_inode_number(const struct SqshInode *inode);

const struct SqshInodeDirectory *
sqsh_data_inode_directory(const struct SqshInode *inode);
const struct SqshInodeDirectoryExt *
sqsh_data_inode_directory_ext(const struct SqshInode *inode);
const struct SqshInodeFile *sqsh_data_inode_file(const struct SqshInode *inode);
const struct SqshInodeFileExt *
sqsh_data_inode_file_ext(const struct SqshInode *inode);
const struct SqshInodeSymlink *
sqsh_data_inode_symlink(const struct SqshInode *inode);
const struct SqshInodeSymlinkExt *
sqsh_data_inode_symlink_ext(const struct SqshInode *inode);
const struct SqshInodeDevice *
sqsh_data_inode_device(const struct SqshInode *inode);
const struct SqshInodeDeviceExt *
sqsh_data_inode_device_ext(const struct SqshInode *inode);
const struct SqshInodeIpc *sqsh_data_inode_ipc(const struct SqshInode *inode);
const struct SqshInodeIpcExt *
sqsh_data_inode_ipc_ext(const struct SqshInode *inode);

uint32_t sqsh_data_inode_directory_block_start(
		const struct SqshInodeDirectory *directory);
uint32_t sqsh_data_inode_directory_hard_link_count(
		const struct SqshInodeDirectory *directory);
uint16_t
sqsh_data_inode_directory_file_size(const struct SqshInodeDirectory *directory);
uint16_t sqsh_data_inode_directory_block_offset(
		const struct SqshInodeDirectory *directory);
uint32_t sqsh_data_inode_directory_parent_inode_number(
		const struct SqshInodeDirectory *directory);

uint32_t sqsh_data_inode_directory_ext_hard_link_count(
		const struct SqshInodeDirectoryExt *directory_ext);
uint32_t sqsh_data_inode_directory_ext_file_size(
		const struct SqshInodeDirectoryExt *directory_ext);
uint32_t sqsh_data_inode_directory_ext_block_start(
		const struct SqshInodeDirectoryExt *directory_ext);
uint32_t sqsh_data_inode_directory_ext_parent_inode_number(
		const struct SqshInodeDirectoryExt *directory_ext);
uint16_t sqsh_data_inode_directory_ext_index_count(
		const struct SqshInodeDirectoryExt *directory_ext);
uint16_t sqsh_data_inode_directory_ext_block_offset(
		const struct SqshInodeDirectoryExt *directory_ext);
uint32_t sqsh_data_inode_directory_ext_xattr_idx(
		const struct SqshInodeDirectoryExt *directory_ext);
const uint8_t *sqsh_data_inode_directory_ext_index(
		const struct SqshInodeDirectoryExt *directory_ext);

uint32_t sqsh_data_inode_directory_index_index(
		const struct SqshInodeDirectoryIndex *directory_index);
uint32_t sqsh_data_inode_directory_index_start(
		const struct SqshInodeDirectoryIndex *directory_index);
uint32_t sqsh_data_inode_directory_index_name_size(
		const struct SqshInodeDirectoryIndex *directory_index);
const uint8_t *sqsh_data_inode_directory_index_name(
		const struct SqshInodeDirectoryIndex *directory_index);

uint32_t sqsh_data_inode_file_blocks_start(const struct SqshInodeFile *file);
uint32_t
sqsh_data_inode_file_fragment_block_index(const struct SqshInodeFile *file);
uint32_t sqsh_data_inode_file_block_offset(const struct SqshInodeFile *file);
uint32_t sqsh_data_inode_file_size(const struct SqshInodeFile *file);
const struct SqshDatablockSize *
sqsh_data_inode_file_block_sizes(const struct SqshInodeFile *file);

uint64_t
sqsh_data_inode_file_ext_blocks_start(const struct SqshInodeFileExt *file_ext);
uint64_t sqsh_data_inode_file_ext_size(const struct SqshInodeFileExt *file_ext);
uint64_t
sqsh_data_inode_file_ext_sparse(const struct SqshInodeFileExt *file_ext);
uint32_t sqsh_data_inode_file_ext_hard_link_count(
		const struct SqshInodeFileExt *file_ext);
uint32_t sqsh_data_inode_file_ext_fragment_block_index(
		const struct SqshInodeFileExt *file_ext);
uint32_t
sqsh_data_inode_file_ext_block_offset(const struct SqshInodeFileExt *file_ext);
uint32_t
sqsh_data_inode_file_ext_xattr_idx(const struct SqshInodeFileExt *file_ext);
const struct SqshDatablockSize *
sqsh_data_inode_file_ext_block_sizes(const struct SqshInodeFileExt *file_ext);

uint32_t sqsh_data_inode_symlink_hard_link_count(
		const struct SqshInodeSymlink *directory);
uint32_t
sqsh_data_inode_symlink_target_size(const struct SqshInodeSymlink *directory);
const uint8_t *
sqsh_data_inode_symlink_target_path(const struct SqshInodeSymlink *directory);

uint32_t sqsh_data_inode_symlink_ext_hard_link_count(
		const struct SqshInodeSymlinkExt *directory);
uint32_t sqsh_data_inode_symlink_ext_target_size(
		const struct SqshInodeSymlinkExt *directory);
const uint8_t *sqsh_data_inode_symlink_ext_target_path(
		const struct SqshInodeSymlinkExt *directory);
uint32_t sqsh_data_inode_symlink_ext_xattr_idx(
		const struct SqshInodeSymlinkExt *directory);

uint32_t
sqsh_data_inode_device_hard_link_count(const struct SqshInodeDevice *device);
uint32_t sqsh_data_inode_device_device(const struct SqshInodeDevice *device);

uint32_t sqsh_data_inode_device_ext_hard_link_count(
		const struct SqshInodeDeviceExt *device);
uint32_t
sqsh_data_inode_device_ext_device(const struct SqshInodeDeviceExt *device);
uint32_t
sqsh_data_inode_device_ext_xattr_idx(const struct SqshInodeDeviceExt *device);

uint32_t sqsh_data_inode_ipc_hard_link_count(const struct SqshInodeIpc *ipc);

uint32_t
sqsh_data_inode_ipc_ext_hard_link_count(const struct SqshInodeIpcExt *ipc);
uint32_t sqsh_data_inode_ipc_ext_xattr_idx(const struct SqshInodeIpcExt *ipc);

#endif /* end of include guard INODE_H */
