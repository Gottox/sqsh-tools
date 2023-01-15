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
 * @file         sqsh_data_private.h
 */

#ifndef SQSH_DATA_PRIVATE_H
#define SQSH_DATA_PRIVATE_H

#include "sqsh_data.h"

#define SQSH_STATIC_ASSERT(cond) _Static_assert(cond, #cond)

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// data/compression_options_internal.c

struct SQSH_UNALIGNED SqshDataCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsGzip) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_GZIP);

struct SQSH_UNALIGNED SqshDataCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsXz) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ);

struct SQSH_UNALIGNED SqshDataCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsLz4) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZ4);

struct SQSH_UNALIGNED SqshDataCompressionOptionsZstd {
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsZstd) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_ZSTD);

struct SQSH_UNALIGNED SqshDataCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsLzo) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZO);

union SqshCompressionOptions {
	struct SqshDataCompressionOptionsGzip gzip;
	struct SqshDataCompressionOptionsXz xz;
	struct SqshDataCompressionOptionsLz4 lz4;
	struct SqshDataCompressionOptionsZstd zstd;
	struct SqshDataCompressionOptionsLzo lzo;
};
SQSH_STATIC_ASSERT(
		sizeof(union SqshCompressionOptions) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS);

////////////////////////////////////////
// data/datablock_internal.c

struct SQSH_UNALIGNED SqshDataDatablockSize {
	uint32_t size;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataDatablockSize) == SQSH_SIZEOF_DATABLOCK_SIZE);

////////////////////////////////////////
// data/directory_internal.c

struct SQSH_UNALIGNED SqshDataDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};

SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataDirectoryEntry) == SQSH_SIZEOF_DIRECTORY_ENTRY);

struct SQSH_UNALIGNED SqshDataDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	// struct SqshDataDirectoryEntry entries[0]; // [count + 1]
};

SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataDirectoryFragment) ==
		SQSH_SIZEOF_DIRECTORY_FRAGMENT);

////////////////////////////////////////
// data/fragment_internal.c

struct SQSH_UNALIGNED SqshDataFragment {
	uint64_t start;
	struct SqshDataDatablockSize size;
	uint32_t unused;
};

SQSH_STATIC_ASSERT(sizeof(struct SqshDataFragment) == SQSH_SIZEOF_FRAGMENT);

////////////////////////////////////////
// data/inode_internal.c

struct SQSH_UNALIGNED SqshDataInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeDirectoryIndex) ==
		SQSH_SIZEOF_INODE_DIRECTORY_INDEX);

struct SQSH_UNALIGNED SqshDataInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeDirectory) == SQSH_SIZEOF_INODE_DIRECTORY);

struct SQSH_UNALIGNED SqshDataInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_start;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	// struct SqshDataInodeDirectoryIndex index[0]; // [index_count]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeDirectoryExt) ==
		SQSH_SIZEOF_INODE_DIRECTORY_EXT);

struct SQSH_UNALIGNED SqshDataInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	// uint32_t block_sizes[0];
};
SQSH_STATIC_ASSERT(sizeof(struct SqshDataInodeFile) == SQSH_SIZEOF_INODE_FILE);

struct SQSH_UNALIGNED SqshDataInodeFileExt {
	uint64_t blocks_start;
	uint64_t file_size;
	uint64_t sparse;
	uint32_t hard_link_count;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t xattr_idx;
	// uint32_t block_sizes[0];
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeFileExt) == SQSH_SIZEOF_INODE_FILE_EXT);

struct SQSH_UNALIGNED SqshDataInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeSymlink) == SQSH_SIZEOF_INODE_SYMLINK);

struct SQSH_UNALIGNED SqshDataInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeSymlinkExt) ==
		SQSH_SIZEOF_INODE_SYMLINK_EXT);

struct SQSH_UNALIGNED SqshDataInodeSymlinkExtTail {
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeSymlinkExtTail) ==
		SQSH_SIZEOF_INODE_SYMLINK_EXT_TAIL);

struct SQSH_UNALIGNED SqshDataInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeDevice) == SQSH_SIZEOF_INODE_DEVICE);

struct SQSH_UNALIGNED SqshDataInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeDeviceExt) == SQSH_SIZEOF_INODE_DEVICE_EXT);

struct SQSH_UNALIGNED SqshDataInodeIpc {
	uint32_t hard_link_count;
};
SQSH_STATIC_ASSERT(sizeof(struct SqshDataInodeIpc) == SQSH_SIZEOF_INODE_IPC);

struct SQSH_UNALIGNED SqshDataInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeIpcExt) == SQSH_SIZEOF_INODE_IPC_EXT);

struct SQSH_UNALIGNED SqshDataInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataInodeHeader) == SQSH_SIZEOF_INODE_HEADER);

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

////////////////////////////////////////
// data/metablock_internal.c

struct SQSH_UNALIGNED SqshDataMetablock {
	uint16_t header;
	// uint8_t data[0];
};

SQSH_STATIC_ASSERT(sizeof(struct SqshDataMetablock) == SQSH_SIZEOF_METABLOCK);

////////////////////////////////////////
// data/superblock_internal.c

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

SQSH_STATIC_ASSERT(sizeof(struct SqshDataSuperblock) == SQSH_SIZEOF_SUPERBLOCK);

////////////////////////////////////////
// data/xattr_internal.c

struct SQSH_UNALIGNED SqshDataXattrKey {
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size - strlen(prefix)];
};
SQSH_STATIC_ASSERT(sizeof(struct SqshDataXattrKey) == SQSH_SIZEOF_XATTR_KEY);

struct SQSH_UNALIGNED SqshDataXattrValue {
	uint32_t value_size;
	// uint8_t value[0]; // [value_size]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrValue) == SQSH_SIZEOF_XATTR_VALUE);

struct SQSH_UNALIGNED SqshDataXattrLookupTable {
	uint64_t xattr_ref;
	uint32_t count;
	uint32_t size;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrLookupTable) ==
		SQSH_SIZEOF_XATTR_LOOKUP_TABLE);

struct SQSH_UNALIGNED SqshDataXattrIdTable {
	uint64_t xattr_table_start;
	uint32_t xattr_ids;
	uint32_t _unused;
	// uint64_t table[0]; // [ceil(xattr_ids / 512.0)]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrIdTable) == SQSH_SIZEOF_XATTR_ID_TABLE);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_DATA_PRIVATE */
