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

enum HsqsInodeType {
	HSQS_INODE_TYPE_BASIC_DIRECTORY = 1,
	HSQS_INODE_TYPE_BASIC_FILE = 2,
	HSQS_INODE_TYPE_BASIC_SYMLINK = 3,
	HSQS_INODE_TYPE_BASIC_BLOCK = 4,
	HSQS_INODE_TYPE_BASIC_CHAR = 5,
	HSQS_INODE_TYPE_BASIC_FIFO = 6,
	HSQS_INODE_TYPE_BASIC_SOCKET = 7,
	HSQS_INODE_TYPE_EXTENDED_DIRECTORY = 8,
	HSQS_INODE_TYPE_EXTENDED_FILE = 9,
	HSQS_INODE_TYPE_EXTENDED_SYMLINK = 10,
	HSQS_INODE_TYPE_EXTENDED_BLOCK = 11,
	HSQS_INODE_TYPE_EXTENDED_CHAR = 12,
	HSQS_INODE_TYPE_EXTENDED_FIFO = 13,
	HSQS_INODE_TYPE_EXTENDED_SOCKET = 14,
};

struct HsqsInodeDirectory;
struct HsqsInodeDirectoryExt;
struct HsqsInodeFile;
struct HsqsInodeFileExt;
struct HsqsInodeSymlink;
struct HsqsInodeSymlinkExt;
struct HsqsInodeSymlinkExtTail;
struct HsqsInodeDevice;
struct HsqsInodeDeviceExt;
struct HsqsInodeIpc;
struct HsqsInodeIpcExt;
struct HsqsInode;

struct HsqsInodeDirectoryIndex;

uint16_t hsqs_data_inode_type(const struct HsqsInode *inode);
uint16_t hsqs_data_inode_permissions(const struct HsqsInode *inode);
uint16_t hsqs_data_inode_uid_idx(const struct HsqsInode *inode);
uint16_t hsqs_data_inode_gid_idx(const struct HsqsInode *inode);
uint32_t hsqs_data_inode_modified_time(const struct HsqsInode *inode);
uint32_t hsqs_data_inode_number(const struct HsqsInode *inode);

const struct HsqsInodeDirectory *
hsqs_data_inode_directory(const struct HsqsInode *inode);
const struct HsqsInodeDirectoryExt *
hsqs_data_inode_directory_ext(const struct HsqsInode *inode);
const struct HsqsInodeFile *hsqs_data_inode_file(const struct HsqsInode *inode);
const struct HsqsInodeFileExt *
hsqs_data_inode_file_ext(const struct HsqsInode *inode);
const struct HsqsInodeSymlink *
hsqs_data_inode_symlink(const struct HsqsInode *inode);
const struct HsqsInodeSymlinkExt *
hsqs_data_inode_symlink_ext(const struct HsqsInode *inode);
const struct HsqsInodeDevice *
hsqs_data_inode_device(const struct HsqsInode *inode);
const struct HsqsInodeDeviceExt *
hsqs_data_inode_device_ext(const struct HsqsInode *inode);
const struct HsqsInodeIpc *hsqs_data_inode_ipc(const struct HsqsInode *inode);
const struct HsqsInodeIpcExt *
hsqs_data_inode_ipc_ext(const struct HsqsInode *inode);

uint32_t hsqs_data_inode_directory_block_start(
		const struct HsqsInodeDirectory *directory);
uint32_t hsqs_data_inode_directory_hard_link_count(
		const struct HsqsInodeDirectory *directory);
uint16_t
hsqs_data_inode_directory_file_size(const struct HsqsInodeDirectory *directory);
uint16_t hsqs_data_inode_directory_block_offset(
		const struct HsqsInodeDirectory *directory);
uint32_t hsqs_data_inode_directory_parent_inode_number(
		const struct HsqsInodeDirectory *directory);

uint32_t hsqs_data_inode_directory_ext_hard_link_count(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint32_t hsqs_data_inode_directory_ext_file_size(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint32_t hsqs_data_inode_directory_ext_block_start(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint32_t hsqs_data_inode_directory_ext_parent_inode_number(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint16_t hsqs_data_inode_directory_ext_index_count(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint16_t hsqs_data_inode_directory_ext_block_offset(
		const struct HsqsInodeDirectoryExt *directory_ext);
uint32_t hsqs_data_inode_directory_ext_xattr_idx(
		const struct HsqsInodeDirectoryExt *directory_ext);
const uint8_t *hsqs_data_inode_directory_ext_index(
		const struct HsqsInodeDirectoryExt *directory_ext);

uint32_t hsqs_data_inode_directory_index_index(
		const struct HsqsInodeDirectoryIndex *directory_index);
uint32_t hsqs_data_inode_directory_index_start(
		const struct HsqsInodeDirectoryIndex *directory_index);
uint32_t hsqs_data_inode_directory_index_name_size(
		const struct HsqsInodeDirectoryIndex *directory_index);
const uint8_t *hsqs_data_inode_directory_index_name(
		const struct HsqsInodeDirectoryIndex *directory_index);

uint32_t hsqs_data_inode_file_blocks_start(const struct HsqsInodeFile *file);
uint32_t
hsqs_data_inode_file_fragment_block_index(const struct HsqsInodeFile *file);
uint32_t hsqs_data_inode_file_block_offset(const struct HsqsInodeFile *file);
uint32_t hsqs_data_inode_file_size(const struct HsqsInodeFile *file);
const struct HsqsDatablockSize *
hsqs_data_inode_file_block_sizes(const struct HsqsInodeFile *file);

uint64_t
hsqs_data_inode_file_ext_blocks_start(const struct HsqsInodeFileExt *file_ext);
uint64_t hsqs_data_inode_file_ext_size(const struct HsqsInodeFileExt *file_ext);
uint64_t
hsqs_data_inode_file_ext_sparse(const struct HsqsInodeFileExt *file_ext);
uint32_t hsqs_data_inode_file_ext_hard_link_count(
		const struct HsqsInodeFileExt *file_ext);
uint32_t hsqs_data_inode_file_ext_fragment_block_index(
		const struct HsqsInodeFileExt *file_ext);
uint32_t
hsqs_data_inode_file_ext_block_offset(const struct HsqsInodeFileExt *file_ext);
uint32_t
hsqs_data_inode_file_ext_xattr_idx(const struct HsqsInodeFileExt *file_ext);
const struct HsqsDatablockSize *
hsqs_data_inode_file_ext_block_sizes(const struct HsqsInodeFileExt *file_ext);

uint32_t hsqs_data_inode_symlink_hard_link_count(
		const struct HsqsInodeSymlink *directory);
uint32_t
hsqs_data_inode_symlink_target_size(const struct HsqsInodeSymlink *directory);
const uint8_t *
hsqs_data_inode_symlink_target_path(const struct HsqsInodeSymlink *directory);

uint32_t hsqs_data_inode_symlink_ext_hard_link_count(
		const struct HsqsInodeSymlinkExt *directory);
uint32_t hsqs_data_inode_symlink_ext_target_size(
		const struct HsqsInodeSymlinkExt *directory);
const uint8_t *hsqs_data_inode_symlink_ext_target_path(
		const struct HsqsInodeSymlinkExt *directory);
uint32_t hsqs_data_inode_symlink_ext_xattr_idx(
		const struct HsqsInodeSymlinkExt *directory);

uint32_t
hsqs_data_inode_device_hard_link_count(const struct HsqsInodeDevice *device);
uint32_t hsqs_data_inode_device_device(const struct HsqsInodeDevice *device);

uint32_t hsqs_data_inode_device_ext_hard_link_count(
		const struct HsqsInodeDeviceExt *device);
uint32_t
hsqs_data_inode_device_ext_device(const struct HsqsInodeDeviceExt *device);
uint32_t
hsqs_data_inode_device_ext_xattr_idx(const struct HsqsInodeDeviceExt *device);

uint32_t hsqs_data_inode_ipc_hard_link_count(const struct HsqsInodeIpc *ipc);

uint32_t
hsqs_data_inode_ipc_ext_hard_link_count(const struct HsqsInodeIpcExt *ipc);
uint32_t hsqs_data_inode_ipc_ext_xattr_idx(const struct HsqsInodeIpcExt *ipc);

#endif /* end of include guard INODE_H */
