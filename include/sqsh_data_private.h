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
 * @file         sqsh_data_private.h
 */

#ifndef SQSH_DATA_PRIVATE_H

#define SQSH_DATA_PRIVATE_H

#include "sqsh_data.h"

// data/compression_options_internal.c

struct SQSH_UNALIGNED SqshCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshCompressionOptionsGzip) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_GZIP);

struct SQSH_UNALIGNED SqshCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshCompressionOptionsXz) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ);

struct SQSH_UNALIGNED SqshCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshCompressionOptionsLz4) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZ4);

struct SQSH_UNALIGNED SqshCompressionOptionsZstd {
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshCompressionOptionsZstd) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_ZSTD);

struct SQSH_UNALIGNED SqshCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshCompressionOptionsLzo) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZO);

union SqshCompressionOptions {
	struct SqshCompressionOptionsGzip gzip;
	struct SqshCompressionOptionsXz xz;
	struct SqshCompressionOptionsLz4 lz4;
	struct SqshCompressionOptionsZstd zstd;
	struct SqshCompressionOptionsLzo lzo;
};
SQSH_STATIC_ASSERT(
		sizeof(union SqshCompressionOptions) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS);

// data/datablock_internal.c

struct SQSH_UNALIGNED SqshDatablockSize {
	uint32_t size;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDatablockSize) == SQSH_SIZEOF_DATABLOCK_SIZE);

// data/directory_internal.c

struct SQSH_UNALIGNED SqshDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};

SQSH_STATIC_ASSERT(
		sizeof(struct SqshDirectoryEntry) == SQSH_SIZEOF_DIRECTORY_ENTRY);

struct SQSH_UNALIGNED SqshDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	// struct SqshDirectoryEntry entries[0]; // [count + 1]
};

SQSH_STATIC_ASSERT(
		sizeof(struct SqshDirectoryFragment) == SQSH_SIZEOF_DIRECTORY_FRAGMENT);

// data/fragment_internal.c

struct SQSH_UNALIGNED SqshFragment {
	uint64_t start;
	struct SqshDatablockSize size;
	uint32_t unused;
};

SQSH_STATIC_ASSERT(sizeof(struct SqshFragment) == SQSH_SIZEOF_FRAGMENT);

// data/inode_internal.c

struct SQSH_UNALIGNED SqshInodeDirectoryIndex {
	uint32_t index;
	uint32_t start;
	uint32_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeDirectoryIndex) ==
		SQSH_SIZEOF_INODE_DIRECTORY_INDEX);

struct SQSH_UNALIGNED SqshInodeDirectory {
	uint32_t block_start;
	uint32_t hard_link_count;
	uint16_t file_size;
	uint16_t block_offset;
	uint32_t parent_inode_number;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeDirectory) == SQSH_SIZEOF_INODE_DIRECTORY);

struct SQSH_UNALIGNED SqshInodeDirectoryExt {
	uint32_t hard_link_count;
	uint32_t file_size;
	uint32_t block_start;
	uint32_t parent_inode_number;
	uint16_t index_count;
	uint16_t block_offset;
	uint32_t xattr_idx;
	// struct SqshInodeDirectoryIndex index[0]; // [index_count]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeDirectoryExt) ==
		SQSH_SIZEOF_INODE_DIRECTORY_EXT);

struct SQSH_UNALIGNED SqshInodeFile {
	uint32_t blocks_start;
	uint32_t fragment_block_index;
	uint32_t block_offset;
	uint32_t file_size;
	// uint32_t block_sizes[0];
};
SQSH_STATIC_ASSERT(sizeof(struct SqshInodeFile) == SQSH_SIZEOF_INODE_FILE);

struct SQSH_UNALIGNED SqshInodeFileExt {
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
		sizeof(struct SqshInodeFileExt) == SQSH_SIZEOF_INODE_FILE_EXT);

struct SQSH_UNALIGNED SqshInodeSymlink {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeSymlink) == SQSH_SIZEOF_INODE_SYMLINK);

struct SQSH_UNALIGNED SqshInodeSymlinkExt {
	uint32_t hard_link_count;
	uint32_t target_size;
	// uint8_t target_path[0]; // [target_size]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeSymlinkExt) == SQSH_SIZEOF_INODE_SYMLINK_EXT);

struct SQSH_UNALIGNED SqshInodeSymlinkExtTail {
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeSymlinkExtTail) ==
		SQSH_SIZEOF_INODE_SYMLINK_EXT_TAIL);

struct SQSH_UNALIGNED SqshInodeDevice {
	uint32_t hard_link_count;
	uint32_t device;
};
SQSH_STATIC_ASSERT(sizeof(struct SqshInodeDevice) == SQSH_SIZEOF_INODE_DEVICE);

struct SQSH_UNALIGNED SqshInodeDeviceExt {
	uint32_t hard_link_count;
	uint32_t device;
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshInodeDeviceExt) == SQSH_SIZEOF_INODE_DEVICE_EXT);

struct SQSH_UNALIGNED SqshInodeIpc {
	uint32_t hard_link_count;
};
SQSH_STATIC_ASSERT(sizeof(struct SqshInodeIpc) == SQSH_SIZEOF_INODE_IPC);

struct SQSH_UNALIGNED SqshInodeIpcExt {
	uint32_t hard_link_count;
	uint32_t xattr_idx;
};
SQSH_STATIC_ASSERT(sizeof(struct SqshInodeIpcExt) == SQSH_SIZEOF_INODE_IPC_EXT);

struct SQSH_UNALIGNED SqshInodeHeader {
	uint16_t type;
	uint16_t permissions;
	uint16_t uid_idx;
	uint16_t gid_idx;
	uint32_t modified_time;
	uint32_t inode_number;
};
SQSH_STATIC_ASSERT(sizeof(struct SqshInodeHeader) == SQSH_SIZEOF_INODE_HEADER);

struct SQSH_UNALIGNED SqshInode {
	struct SqshInodeHeader header;
	union {
		struct SqshInodeDirectory directory;
		struct SqshInodeDirectoryExt directory_ext;
		struct SqshInodeFile file;
		struct SqshInodeFileExt file_ext;
		struct SqshInodeSymlink symlink;
		struct SqshInodeSymlinkExt symlink_ext;
		struct SqshInodeDevice device;
		struct SqshInodeDeviceExt device_ext;
		struct SqshInodeIpc ipc;
		struct SqshInodeIpcExt ipc_ext;
	} data;
};

// data/metablock_internal.c

struct SQSH_UNALIGNED SqshMetablock {
	uint16_t header;
	// uint8_t data[0];
};

SQSH_STATIC_ASSERT(sizeof(struct SqshMetablock) == SQSH_SIZEOF_METABLOCK);

// data/superblock_internal.c

struct SQSH_UNALIGNED SqshSuperblock {
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

SQSH_STATIC_ASSERT(sizeof(struct SqshSuperblock) == SQSH_SIZEOF_SUPERBLOCK);

// data/xattr_internal.c

struct SQSH_UNALIGNED SqshXattrKey {
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size - strlen(prefix)];
};
SQSH_STATIC_ASSERT(sizeof(struct SqshXattrKey) == SQSH_SIZEOF_XATTR_KEY);

struct SQSH_UNALIGNED SqshXattrValue {
	uint32_t value_size;
	// uint8_t value[0]; // [value_size]
};
SQSH_STATIC_ASSERT(sizeof(struct SqshXattrValue) == SQSH_SIZEOF_XATTR_VALUE);

struct SQSH_UNALIGNED SqshXattrLookupTable {
	uint64_t xattr_ref;
	uint32_t count;
	uint32_t size;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshXattrLookupTable) == SQSH_SIZEOF_XATTR_LOOKUP_TABLE);

struct SQSH_UNALIGNED SqshXattrIdTable {
	uint64_t xattr_table_start;
	uint32_t xattr_ids;
	uint32_t _unused;
	// uint64_t table[0]; // [ceil(xattr_ids / 512.0)]
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshXattrIdTable) == SQSH_SIZEOF_XATTR_ID_TABLE);

#endif /* end of include guard SQSH_DATA_PRIVATE */
