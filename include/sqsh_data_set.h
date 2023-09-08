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

#ifndef SQSH_DATA_SET_H
#define SQSH_DATA_SET_H

#include "sqsh_data_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * data/inode_data.c
 */

void
sqsh__data_inode_type_set(struct SqshDataInode *inode, const uint16_t value);
void sqsh__data_inode_permissions_set(
		struct SqshDataInode *inode, const uint16_t value);
void
sqsh__data_inode_uid_idx_set(struct SqshDataInode *inode, const uint16_t value);
void
sqsh__data_inode_gid_idx_set(struct SqshDataInode *inode, const uint16_t value);
void sqsh__data_inode_modified_time_set(
		struct SqshDataInode *inode, const uint32_t value);
void
sqsh__data_inode_number_set(struct SqshDataInode *inode, const uint32_t value);

struct SqshDataInodeDirectory *
sqsh__data_inode_directory_mut(struct SqshDataInode *inode);
struct SqshDataInodeDirectoryExt *
sqsh__data_inode_directory_ext_mut(struct SqshDataInode *inode);
struct SqshDataInodeFile *
sqsh__data_inode_file_mut(struct SqshDataInode *inode);
struct SqshDataInodeFileExt *
sqsh__data_inode_file_ext_mut(struct SqshDataInode *inode);
struct SqshDataInodeSymlink *
sqsh__data_inode_symlink_mut(struct SqshDataInode *inode);
struct SqshDataInodeSymlinkExt *
sqsh__data_inode_symlink_ext_mut(struct SqshDataInode *inode);
struct SqshDataInodeDevice *
sqsh__data_inode_device_mut(struct SqshDataInode *inode);
struct SqshDataInodeDeviceExt *
sqsh__data_inode_device_ext_mut(struct SqshDataInode *inode);
struct SqshDataInodeIpc *sqsh__data_inode_ipc_mut(struct SqshDataInode *inode);
struct SqshDataInodeIpcExt *
sqsh__data_inode_ipc_ext_mut(struct SqshDataInode *inode);

void sqsh__data_inode_directory_block_start_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value);
void sqsh__data_inode_directory_hard_link_count_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value);
void sqsh__data_inode_directory_file_size_set(
		struct SqshDataInodeDirectory *directory, const uint16_t value);
void sqsh__data_inode_directory_block_offset_set(
		struct SqshDataInodeDirectory *directory, const uint16_t value);
void sqsh__data_inode_directory_parent_inode_number_set(
		struct SqshDataInodeDirectory *directory, const uint32_t value);
void sqsh__data_inode_directory_ext_hard_link_count_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value);
void sqsh__data_inode_directory_ext_file_size_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value);
void sqsh__data_inode_directory_ext_block_start_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value);
void sqsh__data_inode_directory_ext_parent_inode_number_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value);
void sqsh__data_inode_directory_ext_index_count_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint16_t value);
void sqsh__data_inode_directory_ext_block_offset_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint16_t value);
void sqsh__data_inode_directory_ext_xattr_idx_set(
		struct SqshDataInodeDirectoryExt *directory_ext, const uint32_t value);
void sqsh__data_inode_directory_index_index_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value);
void sqsh__data_inode_directory_index_start_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value);
void sqsh__data_inode_directory_index_name_size_set(
		struct SqshDataInodeDirectoryIndex *directory_index,
		const uint32_t value);
void sqsh__data_inode_file_blocks_start_set(
		struct SqshDataInodeFile *file, const uint32_t value);
void sqsh__data_inode_file_fragment_block_index_set(
		struct SqshDataInodeFile *file, const uint32_t value);
void sqsh__data_inode_file_block_offset_set(
		struct SqshDataInodeFile *file, const uint32_t value);
void sqsh__data_inode_file_size_set(
		struct SqshDataInodeFile *file, const uint32_t value);
void sqsh__data_inode_file_ext_blocks_start_set(
		struct SqshDataInodeFileExt *file_ext, const uint64_t value);
void sqsh__data_inode_file_ext_size_set(
		struct SqshDataInodeFileExt *file_ext, const uint64_t value);
void sqsh__data_inode_file_ext_sparse_set(
		struct SqshDataInodeFileExt *file_ext, const uint64_t value);
void sqsh__data_inode_file_ext_hard_link_count_set(
		struct SqshDataInodeFileExt *file_ext, const uint32_t value);
void sqsh__data_inode_file_ext_fragment_block_index_set(
		struct SqshDataInodeFileExt *file_ext, const uint32_t value);
void sqsh__data_inode_file_ext_block_offset_set(
		struct SqshDataInodeFileExt *file_ext, const uint32_t value);
void sqsh__data_inode_file_ext_xattr_idx_set(
		struct SqshDataInodeFileExt *file_ext, const uint32_t value);
void sqsh__data_inode_symlink_hard_link_count_set(
		struct SqshDataInodeSymlink *symlink, const uint32_t value);
void sqsh__data_inode_symlink_target_size_set(
		struct SqshDataInodeSymlink *symlink, const uint32_t value);
void sqsh__data_inode_symlink_ext_hard_link_count_set(
		struct SqshDataInodeSymlinkExt *symlink_ext, const uint32_t value);
void sqsh__data_inode_symlink_ext_target_size_set(
		struct SqshDataInodeSymlinkExt *symlink_ext, const uint32_t value);
void sqsh__data_inode_symlink_ext_tail_xattr_idx_set(
		struct SqshDataInodeSymlinkExtTail *symlink_ext_tail,
		const uint32_t value);
void sqsh__data_inode_device_hard_link_count_set(
		struct SqshDataInodeDevice *device, const uint32_t value);
void sqsh__data_inode_device_device_set(
		struct SqshDataInodeDevice *device, const uint32_t value);
void sqsh__data_inode_device_ext_hard_link_count_set(
		struct SqshDataInodeDeviceExt *device_ext, const uint32_t value);
void sqsh__data_inode_device_ext_device_set(
		struct SqshDataInodeDeviceExt *device_ext, const uint32_t value);
void sqsh__data_inode_device_ext_xattr_idx_set(
		struct SqshDataInodeDeviceExt *device_ext, const uint32_t value);
void sqsh__data_inode_ipc_hard_link_count_set(
		struct SqshDataInodeIpc *ipc, const uint32_t value);
void sqsh__data_inode_ipc_ext_hard_link_count_set(
		struct SqshDataInodeIpcExt *ipc_ext, const uint32_t value);
void sqsh__data_inode_ipc_ext_xattr_idx_set(
		struct SqshDataInodeIpcExt *ipc_ext, const uint32_t value);

/***************************************
 * data/superblock_data.c
 */

void sqsh__data_superblock_magic_set(
		struct SqshDataSuperblock *superblock, const uint32_t value);
void sqsh__data_superblock_inode_count_set(
		struct SqshDataSuperblock *superblock, const uint32_t value);
void sqsh__data_superblock_modification_time_set(
		struct SqshDataSuperblock *superblock, const uint32_t value);
void sqsh__data_superblock_block_size_set(
		struct SqshDataSuperblock *superblock, const uint32_t value);
void sqsh__data_superblock_fragment_entry_count_set(
		struct SqshDataSuperblock *superblock, const uint32_t value);
void sqsh__data_superblock_compression_id_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_block_log_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_flags_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_id_count_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_version_major_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_version_minor_set(
		struct SqshDataSuperblock *superblock, const uint16_t value);
void sqsh__data_superblock_root_inode_ref_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_bytes_used_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_id_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_xattr_id_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_inode_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_directory_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_fragment_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);
void sqsh__data_superblock_export_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value);

/***************************************
 * data/metablock_data.c
 */

void sqsh__data_metablock_is_compressed_set(
		struct SqshDataMetablock *metablock, const int value);
void sqsh__data_metablock_size_set(
		struct SqshDataMetablock *metablock, const uint16_t value);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_DATA_SET_H */
