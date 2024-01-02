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

#ifndef SQSH_DATA_PRIVATE_H
#define SQSH_DATA_PRIVATE_H

#define SQSH_UNALIGNED __attribute__((packed, aligned(1)))

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * data/compression_options_data.c
 */

struct SQSH_UNALIGNED SqshDataCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};

struct SQSH_UNALIGNED SqshDataCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};

struct SQSH_UNALIGNED SqshDataCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};

struct SQSH_UNALIGNED SqshDataCompressionOptionsZstd {
	uint32_t compression_level;
};

struct SQSH_UNALIGNED SqshDataCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};

union SqshDataCompressionOptions {
	struct SqshDataCompressionOptionsGzip gzip;
	struct SqshDataCompressionOptionsXz xz;
	struct SqshDataCompressionOptionsLz4 lz4;
	struct SqshDataCompressionOptionsZstd zstd;
	struct SqshDataCompressionOptionsLzo lzo;
};

SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_gzip_compression_level(
		const union SqshDataCompressionOptions *options);
SQSH_NO_EXPORT uint16_t sqsh_compression_data_options_gzip_window_size(
		const union SqshDataCompressionOptions *options);
SQSH_NO_EXPORT uint16_t sqsh_compression_data_options_gzip_strategies(
		const union SqshDataCompressionOptions *options);

SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_xz_dictionary_size(
		const union SqshDataCompressionOptions *options);
SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_xz_filters(
		const union SqshDataCompressionOptions *options);

SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_lz4_version(
		const union SqshDataCompressionOptions *options);
SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_lz4_flags(
		const union SqshDataCompressionOptions *options);

SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_zstd_compression_level(
		const union SqshDataCompressionOptions *options);

SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_lzo_algorithm(
		const union SqshDataCompressionOptions *options);
SQSH_NO_EXPORT uint32_t sqsh_compression_data_options_lzo_compression_level(
		const union SqshDataCompressionOptions *options);

/***************************************
 * data/directory_data.c
 */

struct SQSH_UNALIGNED SqshDataDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	/* uint8_t name[0]; // [name_size + 1] */
};

struct SQSH_UNALIGNED SqshDataDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	/* struct SqshDataDirectoryEntry entries[0]; // [count + 1] */
};

SQSH_NO_EXPORT uint16_t
sqsh__data_directory_entry_offset(const struct SqshDataDirectoryEntry *entry);
SQSH_NO_EXPORT int16_t sqsh__data_directory_entry_inode_offset(
		const struct SqshDataDirectoryEntry *entry);
SQSH_NO_EXPORT uint16_t
sqsh__data_directory_entry_type(const struct SqshDataDirectoryEntry *entry);
SQSH_NO_EXPORT uint16_t sqsh__data_directory_entry_name_size(
		const struct SqshDataDirectoryEntry *entry);
SQSH_NO_EXPORT const uint8_t *
sqsh__data_directory_entry_name(const struct SqshDataDirectoryEntry *entry);

SQSH_NO_EXPORT uint32_t sqsh__data_directory_fragment_count(
		const struct SqshDataDirectoryFragment *fragment);
SQSH_NO_EXPORT uint32_t sqsh__data_directory_fragment_start(
		const struct SqshDataDirectoryFragment *fragment);
SQSH_NO_EXPORT uint32_t sqsh__data_directory_fragment_inode_number(
		const struct SqshDataDirectoryFragment *fragment);

/***************************************
 * data/fragment_data.c
 */

struct SQSH_UNALIGNED SqshDataFragment {
	uint64_t start;
	uint32_t size_info;
	uint32_t unused;
};

SQSH_NO_EXPORT uint64_t
sqsh__data_fragment_start(const struct SqshDataFragment *fragment);
SQSH_NO_EXPORT uint32_t
sqsh__data_fragment_size_info(const struct SqshDataFragment *fragment);

/***************************************
 * data/inode_data.c
 */

enum SqshDataInodeType {
	SQSH_INODE_TYPE_BASIC_DIRECTORY = 1,
	SQSH_INODE_TYPE_BASIC_FILE = 2,
	SQSH_INODE_TYPE_BASIC_SYMLINK = 3,
	SQSH_INODE_TYPE_BASIC_BLOCK = 4,
	SQSH_INODE_TYPE_BASIC_CHAR = 5,
	SQSH_INODE_TYPE_BASIC_FIFO = 6,
	SQSH_INODE_TYPE_BASIC_SOCKET = 7,
	SQSH_INODE_TYPE_EXTENDED_DIRECTORY = 8,
	SQSH_INODE_TYPE_EXTENDED_FILE = 9,
	SQSH_INODE_TYPE_EXTENDED_SYMLINK = 10,
	SQSH_INODE_TYPE_EXTENDED_BLOCK = 11,
	SQSH_INODE_TYPE_EXTENDED_CHAR = 12,
	SQSH_INODE_TYPE_EXTENDED_FIFO = 13,
	SQSH_INODE_TYPE_EXTENDED_SOCKET = 14
};

struct SQSH_UNALIGNED SqshDataInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	/* uint8_t name[0]; // [name_size + 1] */
};

struct SQSH_UNALIGNED SqshDataInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};

struct SQSH_UNALIGNED SqshDataInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_start;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	/* struct SqshDataInodeDirectoryIndex index[0]; // [index_count] */
};

struct SQSH_UNALIGNED SqshDataInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	/* uint32_t block_sizes[0]; */
};

struct SQSH_UNALIGNED SqshDataInodeFileExt {
	uint64_t blocks_start;
	uint64_t file_size;
	uint64_t sparse;
	uint32_t hard_link_count;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t xattr_idx;
	/* uint32_t block_sizes[0]; */
};

struct SQSH_UNALIGNED SqshDataInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	/* uint8_t target_path[0]; // [target_size] */
};

struct SQSH_UNALIGNED SqshDataInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	/* uint8_t target_path[0]; // [target_size] */
};

struct SQSH_UNALIGNED SqshDataInodeSymlinkExtTail {
	uint32_t xattr_idx;
};

struct SQSH_UNALIGNED SqshDataInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};

struct SQSH_UNALIGNED SqshDataInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};

struct SQSH_UNALIGNED SqshDataInodeIpc {
	uint32_t hard_link_count;
};

struct SQSH_UNALIGNED SqshDataInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};

struct SQSH_UNALIGNED SqshDataInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};

struct SQSH_UNALIGNED SqshDataInode {
	struct SqshDataInodeHeader header;
	union {
		struct SqshDataInodeDirectory directory;
		struct SqshDataInodeDirectoryExt directory_ext;
		struct SqshDataInodeFile file;
		struct SqshDataInodeFileExt file_ext;
		struct SqshDataInodeSymlink symlink;
		struct SqshDataInodeSymlinkExt symlink_ext;
		struct SqshDataInodeDevice device;
		struct SqshDataInodeDeviceExt device_ext;
		struct SqshDataInodeIpc ipc;
		struct SqshDataInodeIpcExt ipc_ext;
	} data;
};

SQSH_NO_EXPORT uint16_t
sqsh__data_inode_type(const struct SqshDataInode *inode);
SQSH_NO_EXPORT uint16_t
sqsh__data_inode_permissions(const struct SqshDataInode *inode);
SQSH_NO_EXPORT uint16_t
sqsh__data_inode_uid_idx(const struct SqshDataInode *inode);
SQSH_NO_EXPORT uint16_t
sqsh__data_inode_gid_idx(const struct SqshDataInode *inode);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_modified_time(const struct SqshDataInode *inode);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_number(const struct SqshDataInode *inode);

SQSH_NO_EXPORT const struct SqshDataInodeDirectory *
sqsh__data_inode_directory(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeDirectoryExt *
sqsh__data_inode_directory_ext(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeFile *
sqsh__data_inode_file(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeFileExt *
sqsh__data_inode_file_ext(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeSymlink *
sqsh__data_inode_symlink(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeSymlinkExt *
sqsh__data_inode_symlink_ext(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeDevice *
sqsh__data_inode_device(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeDeviceExt *
sqsh__data_inode_device_ext(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeIpc *
sqsh__data_inode_ipc(const struct SqshDataInode *inode);
SQSH_NO_EXPORT const struct SqshDataInodeIpcExt *
sqsh__data_inode_ipc_ext(const struct SqshDataInode *inode);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_block_start(
		const struct SqshDataInodeDirectory *directory);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_hard_link_count(
		const struct SqshDataInodeDirectory *directory);
SQSH_NO_EXPORT uint16_t sqsh__data_inode_directory_file_size(
		const struct SqshDataInodeDirectory *directory);
SQSH_NO_EXPORT uint16_t sqsh__data_inode_directory_block_offset(
		const struct SqshDataInodeDirectory *directory);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_parent_inode_number(
		const struct SqshDataInodeDirectory *directory);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_ext_hard_link_count(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_ext_file_size(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_ext_block_start(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_ext_parent_inode_number(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint16_t sqsh__data_inode_directory_ext_index_count(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint16_t sqsh__data_inode_directory_ext_block_offset(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_ext_xattr_idx(
		const struct SqshDataInodeDirectoryExt *directory_ext);
SQSH_NO_EXPORT const uint8_t *sqsh__data_inode_directory_ext_index(
		const struct SqshDataInodeDirectoryExt *directory_ext);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_index_index(
		const struct SqshDataInodeDirectoryIndex *directory_index);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_index_start(
		const struct SqshDataInodeDirectoryIndex *directory_index);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_directory_index_name_size(
		const struct SqshDataInodeDirectoryIndex *directory_index);
SQSH_NO_EXPORT const uint8_t *sqsh__data_inode_directory_index_name(
		const struct SqshDataInodeDirectoryIndex *directory_index);

SQSH_NO_EXPORT uint32_t
sqsh__data_inode_file_blocks_start(const struct SqshDataInodeFile *file);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_fragment_block_index(
		const struct SqshDataInodeFile *file);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_file_block_offset(const struct SqshDataInodeFile *file);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_file_size(const struct SqshDataInodeFile *file);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_block_size_info(
		const struct SqshDataInodeFile *file, sqsh_index_t index);

SQSH_NO_EXPORT uint64_t sqsh__data_inode_file_ext_blocks_start(
		const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint64_t
sqsh__data_inode_file_ext_size(const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint64_t
sqsh__data_inode_file_ext_sparse(const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_ext_hard_link_count(
		const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_ext_fragment_block_index(
		const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_ext_block_offset(
		const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_ext_xattr_idx(
		const struct SqshDataInodeFileExt *file_ext);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_file_ext_block_size_info(
		const struct SqshDataInodeFileExt *file_ext, sqsh_index_t index);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_symlink_hard_link_count(
		const struct SqshDataInodeSymlink *directory);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_symlink_target_size(
		const struct SqshDataInodeSymlink *directory);
SQSH_NO_EXPORT const uint8_t *sqsh__data_inode_symlink_target_path(
		const struct SqshDataInodeSymlink *directory);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_symlink_ext_hard_link_count(
		const struct SqshDataInodeSymlinkExt *directory);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_symlink_ext_target_size(
		const struct SqshDataInodeSymlinkExt *directory);
SQSH_NO_EXPORT const uint8_t *sqsh__data_inode_symlink_ext_target_path(
		const struct SqshDataInodeSymlinkExt *directory);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_symlink_ext_xattr_idx(
		const struct SqshDataInodeSymlinkExt *directory);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_device_hard_link_count(
		const struct SqshDataInodeDevice *device);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_device_device(const struct SqshDataInodeDevice *device);

SQSH_NO_EXPORT uint32_t sqsh__data_inode_device_ext_hard_link_count(
		const struct SqshDataInodeDeviceExt *device);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_device_ext_device(const struct SqshDataInodeDeviceExt *device);
SQSH_NO_EXPORT uint32_t sqsh__data_inode_device_ext_xattr_idx(
		const struct SqshDataInodeDeviceExt *device);

SQSH_NO_EXPORT uint32_t
sqsh__data_inode_ipc_hard_link_count(const struct SqshDataInodeIpc *ipc);

SQSH_NO_EXPORT uint32_t
sqsh__data_inode_ipc_ext_hard_link_count(const struct SqshDataInodeIpcExt *ipc);
SQSH_NO_EXPORT uint32_t
sqsh__data_inode_ipc_ext_xattr_idx(const struct SqshDataInodeIpcExt *ipc);

/***************************************
 * data/metablock_data.c
 */

#define SQSH_METABLOCK_BLOCK_SIZE 8192

struct SQSH_UNALIGNED SqshDataMetablock {
	uint16_t header;
	/* uint8_t data[0]; */
};

SQSH_NO_EXPORT int
sqsh__data_metablock_is_compressed(const struct SqshDataMetablock *metablock);

SQSH_NO_EXPORT uint16_t
sqsh__data_metablock_size(const struct SqshDataMetablock *metablock);

/***************************************
 * data/superblock_data.c
 */

#define SQSH_SUPERBLOCK_MAGIC 0x73717368

struct SQSH_UNALIGNED SqshDataSuperblock {
	uint32_t magic;
	uint32_t inode_count;
	uint32_t modification_time;
	uint32_t block_size;
	uint32_t fragment_entry_count;
	uint16_t compression_id;
	uint16_t block_log;
	uint16_t flags;
	uint16_t id_count;
	uint16_t version_major;
	uint16_t version_minor;
	uint64_t root_inode_ref;
	uint64_t bytes_used;
	uint64_t id_table_start;
	uint64_t xattr_id_table_start;
	uint64_t inode_table_start;
	uint64_t directory_table_start;
	uint64_t fragment_table_start;
	uint64_t export_table_start;
};

SQSH_NO_EXPORT uint32_t
sqsh__data_superblock_magic(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint32_t
sqsh__data_superblock_inode_count(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint32_t sqsh__data_superblock_modification_time(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint32_t
sqsh__data_superblock_block_size(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint32_t sqsh__data_superblock_fragment_entry_count(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t sqsh__data_superblock_compression_id(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t
sqsh__data_superblock_block_log(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t
sqsh__data_superblock_flags(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t
sqsh__data_superblock_id_count(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t sqsh__data_superblock_version_major(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint16_t sqsh__data_superblock_version_minor(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_root_inode_ref(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t
sqsh__data_superblock_bytes_used(const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_id_table_start(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_xattr_id_table_start(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_inode_table_start(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_directory_table_start(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_fragment_table_start(
		const struct SqshDataSuperblock *superblock);
SQSH_NO_EXPORT uint64_t sqsh__data_superblock_export_table_start(
		const struct SqshDataSuperblock *superblock);

/***************************************
 * data/xattr_data.c
 */

struct SQSH_UNALIGNED SqshDataXattrKey {
	uint16_t type;
	uint16_t name_size;
	/* uint8_t name[0]; // [name_size - strlen(prefix)]; */
};

struct SQSH_UNALIGNED SqshDataXattrValue {
	uint32_t value_size;
	/* uint8_t value[0]; // [value_size] */
};

struct SQSH_UNALIGNED SqshDataXattrLookupTable {
	uint64_t xattr_ref;
	uint32_t count;
	uint32_t size;
};

struct SQSH_UNALIGNED SqshDataXattrIdTable {
	uint64_t xattr_table_start;
	uint32_t xattr_ids;
	uint32_t _unused;
	/* uint64_t table[0]; // [ceil(xattr_ids / 512.0)] */
};

SQSH_NO_EXPORT uint16_t
sqsh__data_xattr_key_type(const struct SqshDataXattrKey *xattr_key);
SQSH_NO_EXPORT uint16_t
sqsh__data_xattr_key_name_size(const struct SqshDataXattrKey *xattr_key);
SQSH_NO_EXPORT const uint8_t *
sqsh__data_xattr_key_name(const struct SqshDataXattrKey *xattr_key);

SQSH_NO_EXPORT uint32_t
sqsh__data_xattr_value_size(const struct SqshDataXattrValue *xattr_value);
SQSH_NO_EXPORT uint64_t
sqsh__data_xattr_value_ref(const struct SqshDataXattrValue *xattr_value);
SQSH_NO_EXPORT const uint8_t *
sqsh__data_xattr_value(const struct SqshDataXattrValue *xattr_value);

SQSH_NO_EXPORT uint64_t sqsh__data_xattr_lookup_table_xattr_ref(
		const struct SqshDataXattrLookupTable *lookup_table);
SQSH_NO_EXPORT uint32_t sqsh__data_xattr_lookup_table_count(
		const struct SqshDataXattrLookupTable *lookup_table);
SQSH_NO_EXPORT uint32_t sqsh__data_xattr_lookup_table_size(
		const struct SqshDataXattrLookupTable *lookup_table);

SQSH_NO_EXPORT uint64_t sqsh__data_xattr_id_table_xattr_table_start(
		const struct SqshDataXattrIdTable *xattr_id_table);
SQSH_NO_EXPORT uint32_t sqsh__data_xattr_id_table_xattr_ids(
		const struct SqshDataXattrIdTable *xattr_id_table);
SQSH_NO_EXPORT const uint64_t *
sqsh__data_xattr_id_table(const struct SqshDataXattrIdTable *xattr_id_table);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_DATA_PRIVATE_H */
