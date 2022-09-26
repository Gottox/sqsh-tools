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
 * @file         superblock.h
 */

#ifndef HSQS_SUPERBLOCK_H

#define HSQS_SUPERBLOCK_H

#include <stddef.h>
#include <stdint.h>

#define HSQS_SIZEOF_SUPERBLOCK 96

enum SqshSuperblockCompressionId {
	HSQS_COMPRESSION_NONE = 0,
	HSQS_COMPRESSION_GZIP = 1,
	HSQS_COMPRESSION_LZMA = 2,
	HSQS_COMPRESSION_LZO = 3,
	HSQS_COMPRESSION_XZ = 4,
	HSQS_COMPRESSION_LZ4 = 5,
	HSQS_COMPRESSION_ZSTD = 6,
};

enum SqshSuperblockFlags {
	HSQS_SUPERBLOCK_UNCOMPRESSED_INODES = 0x0001,
	HSQS_SUPERBLOCK_UNCOMPRESSED_DATA = 0x0002,
	HSQS_SUPERBLOCK_CHECK = 0x0004,
	HSQS_SUPERBLOCK_UNCOMPRESSED_FRAGMENTS = 0x0008,
	HSQS_SUPERBLOCK_NO_FRAGMENTS = 0x0010,
	HSQS_SUPERBLOCK_ALWAYS_FRAGMENTS = 0x0020,
	HSQS_SUPERBLOCK_DUPLICATES = 0x0040,
	HSQS_SUPERBLOCK_EXPORTABLE = 0x0080,
	HSQS_SUPERBLOCK_UNCOMPRESSED_XATTRS = 0x0100,
	HSQS_SUPERBLOCK_NO_XATTRS = 0x0200,
	HSQS_SUPERBLOCK_COMPRESSOR_OPTIONS = 0x0400,
	HSQS_SUPERBLOCK_UNCOMPRESSED_IDS = 0x0800,
};

struct SqshSuperblock;

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

#endif /* end of include guard HSQS_SUPERBLOCK_H */
