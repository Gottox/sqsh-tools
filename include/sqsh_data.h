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

#ifndef SQSH_DATA_H
#define SQSH_DATA_H

#include "sqsh_common.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// data/compression_options_data.c

#define SQSH_SIZEOF_COMPRESSION_OPTIONS_GZIP 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_LZ4 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_ZSTD 4
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_LZO 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS 8

struct SQSH_UNALIGNED SqshCompressionOptionsGzip;

struct SQSH_UNALIGNED SqshCompressionOptionsXz;

struct SQSH_UNALIGNED SqshCompressionOptionsLz4;

struct SQSH_UNALIGNED SqshCompressionOptionsZstd;

struct SQSH_UNALIGNED SqshCompressionOptionsLzo;

union SqshCompressionOptions;

uint32_t sqsh_data_compression_options_gzip_compression_level(
		const union SqshCompressionOptions *options);
uint16_t sqsh_data_compression_options_gzip_window_size(
		const union SqshCompressionOptions *options);
uint16_t sqsh_data_compression_options_gzip_strategies(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_xz_dictionary_size(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_xz_filters(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_lz4_version(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_lz4_flags(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_zstd_compression_level(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_lzo_algorithm(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_lzo_compression_level(
		const union SqshCompressionOptions *options);

// data/datablock_data.c

#define SQSH_SIZEOF_DATABLOCK_SIZE 4

struct SQSH_UNALIGNED SqshDatablockSize;

uint32_t
sqsh_data_datablock_size(const struct SqshDatablockSize *datablock_size);
bool sqsh_data_datablock_is_compressed(
		const struct SqshDatablockSize *datablock_size);

// data/directory_data.c

#define SQSH_SIZEOF_DIRECTORY_FRAGMENT 12
#define SQSH_SIZEOF_DIRECTORY_ENTRY 8

struct SQSH_UNALIGNED SqshDirectoryEntry;

struct SQSH_UNALIGNED SqshDirectoryFragment;

uint16_t
sqsh_data_directory_entry_offset(const struct SqshDirectoryEntry *entry);
int16_t
sqsh_data_directory_entry_inode_offset(const struct SqshDirectoryEntry *entry);
uint16_t sqsh_data_directory_entry_type(const struct SqshDirectoryEntry *entry);
uint16_t
sqsh_data_directory_entry_name_size(const struct SqshDirectoryEntry *entry);
const uint8_t *
sqsh_data_directory_entry_name(const struct SqshDirectoryEntry *entry);

uint32_t sqsh_data_directory_fragment_count(
		const struct SqshDirectoryFragment *fragment);
uint32_t sqsh_data_directory_fragment_start(
		const struct SqshDirectoryFragment *fragment);
uint32_t sqsh_data_directory_fragment_inode_number(
		const struct SqshDirectoryFragment *fragment);
const struct SqshDirectoryEntry *sqsh_data_directory_fragment_entries(
		const struct SqshDirectoryFragment *fragment);

// data/fragment_data.c

#define SQSH_SIZEOF_FRAGMENT 16

struct SQSH_UNALIGNED SqshFragment;

uint64_t sqsh_data_fragment_start(const struct SqshFragment *fragment);
const struct SqshDatablockSize *
sqsh_data_fragment_size_info(const struct SqshFragment *fragment);
uint32_t sqsh_data_fragment_is_compressed(const struct SqshFragment *fragment);

// data/inode_data.c

#define SQSH_SIZEOF_INODE_DIRECTORY_INDEX 12
#define SQSH_SIZEOF_INODE_DIRECTORY 16
#define SQSH_SIZEOF_INODE_DIRECTORY_EXT 24
#define SQSH_SIZEOF_INODE_FILE 16
#define SQSH_SIZEOF_INODE_FILE_EXT 40
#define SQSH_SIZEOF_INODE_SYMLINK 8
#define SQSH_SIZEOF_INODE_SYMLINK_EXT 8
#define SQSH_SIZEOF_INODE_SYMLINK_EXT_TAIL 4
#define SQSH_SIZEOF_INODE_DEVICE 8
#define SQSH_SIZEOF_INODE_DEVICE_EXT 12
#define SQSH_SIZEOF_INODE_IPC 4
#define SQSH_SIZEOF_INODE_IPC_EXT 8
#define SQSH_SIZEOF_INODE_HEADER 16

struct SQSH_UNALIGNED SqshInodeDirectory;
struct SQSH_UNALIGNED SqshInodeDirectoryExt;
struct SQSH_UNALIGNED SqshInodeFile;
struct SQSH_UNALIGNED SqshInodeFileExt;
struct SQSH_UNALIGNED SqshInodeSymlink;
struct SQSH_UNALIGNED SqshInodeSymlinkExt;
struct SQSH_UNALIGNED SqshInodeSymlinkExtTail;
struct SQSH_UNALIGNED SqshInodeDevice;
struct SQSH_UNALIGNED SqshInodeDeviceExt;
struct SQSH_UNALIGNED SqshInodeIpc;
struct SQSH_UNALIGNED SqshInodeIpcExt;
struct SQSH_UNALIGNED SqshInode;

struct SQSH_UNALIGNED SqshInodeDirectoryIndex;

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

// data/metablock_data.c

#define SQSH_SIZEOF_METABLOCK 2

struct SQSH_UNALIGNED SqshMetablock;

int sqsh_data_metablock_is_compressed(const struct SqshMetablock *metablock);

const uint8_t *sqsh_data_metablock_data(const struct SqshMetablock *metablock);

size_t sqsh_data_metablock_size(const struct SqshMetablock *metablock);

// data/superblock_data.c

#define SQSH_SIZEOF_SUPERBLOCK 96

struct SQSH_UNALIGNED SqshSuperblock;

int
sqsh_data_superblock_init(const struct SqshSuperblock *superblock, size_t size);

uint32_t sqsh_data_superblock_magic(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_inode_count(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_modification_time(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_block_size(const struct SqshSuperblock *superblock);
uint32_t sqsh_data_superblock_fragment_entry_count(
		const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_compression_id(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_block_log(const struct SqshSuperblock *superblock);
uint16_t sqsh_data_superblock_flags(const struct SqshSuperblock *superblock);
uint16_t sqsh_data_superblock_id_count(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_version_major(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_version_minor(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_root_inode_ref(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_bytes_used(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_id_table_start(const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_xattr_id_table_start(
		const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_inode_table_start(const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_directory_table_start(
		const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_fragment_table_start(
		const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_export_table_start(
		const struct SqshSuperblock *superblock);

// data/xattr_data.c

#define SQSH_SIZEOF_XATTR_KEY 4
#define SQSH_SIZEOF_XATTR_VALUE 4
#define SQSH_SIZEOF_XATTR_LOOKUP_TABLE 16
#define SQSH_SIZEOF_XATTR_ID_TABLE 16

struct SQSH_UNALIGNED SqshXattrKey;

struct SQSH_UNALIGNED SqshXattrValue;

struct SQSH_UNALIGNED SqshXattrLookupTable;

struct SQSH_UNALIGNED SqshXattrIdTable;

uint16_t sqsh_data_xattr_key_type(const struct SqshXattrKey *xattr_key);
uint16_t sqsh_data_xattr_key_name_size(const struct SqshXattrKey *xattr_key);
const uint8_t *sqsh_data_xattr_key_name(const struct SqshXattrKey *xattr_key);

uint32_t sqsh_data_xattr_value_size(const struct SqshXattrValue *xattr_value);
uint64_t sqsh_data_xattr_value_ref(const struct SqshXattrValue *xattr_value);
const uint8_t *sqsh_data_xattr_value(const struct SqshXattrValue *xattr_value);

uint64_t sqsh_data_xattr_lookup_table_xattr_ref(
		const struct SqshXattrLookupTable *lookup_table);
uint32_t sqsh_data_xattr_lookup_table_count(
		const struct SqshXattrLookupTable *lookup_table);
uint32_t sqsh_data_xattr_lookup_table_size(
		const struct SqshXattrLookupTable *lookup_table);

uint64_t sqsh_data_xattr_id_table_xattr_table_start(
		const struct SqshXattrIdTable *xattr_id_table);
uint32_t sqsh_data_xattr_id_table_xattr_ids(
		const struct SqshXattrIdTable *xattr_id_table);
const uint64_t *
sqsh_data_xattr_id_table(const struct SqshXattrIdTable *xattr_id_table);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_DATA_H */
