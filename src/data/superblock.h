/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock
 * @created     : Friday Apr 30, 2021 12:36:57 CEST
 */

#ifndef SQUASH_SUPERBLOCK_H

#define SQUASH_SUPERBLOCK_H

#include <stddef.h>
#include <stdint.h>

#define SQUASH_SIZEOF_SUPERBLOCK 96

enum SquashSuperblockCompressionId {
	SQUASH_COMPRESSION_NONE = 0,
	SQUASH_COMPRESSION_GZIP = 1,
	SQUASH_COMPRESSION_LZMA = 2,
	SQUASH_COMPRESSION_LZO = 3,
	SQUASH_COMPRESSION_XZ = 4,
	SQUASH_COMPRESSION_LZ4 = 5,
	SQUASH_COMPRESSION_ZSTD = 6,
};

enum SquashSuperblockFlags {
	SQUASH_SUPERBLOCK_UNCOMPRESSED_INODES = 0x0001,
	SQUASH_SUPERBLOCK_UNCOMPRESSED_DATA = 0x0002,
	SQUASH_SUPERBLOCK_CHECK = 0x0004,
	SQUASH_SUPERBLOCK_UNCOMPRESSED_FRAGMENTS = 0x0008,
	SQUASH_SUPERBLOCK_NO_FRAGMENTS = 0x0010,
	SQUASH_SUPERBLOCK_ALWAYS_FRAGMENTS = 0x0020,
	SQUASH_SUPERBLOCK_DUPLICATES = 0x0040,
	SQUASH_SUPERBLOCK_EXPORTABLE = 0x0080,
	SQUASH_SUPERBLOCK_UNCOMPRESSED_XATTRS = 0x0100,
	SQUASH_SUPERBLOCK_NO_XATTRS = 0x0200,
	SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS = 0x0400,
	SQUASH_SUPERBLOCK_UNCOMPRESSED_IDS = 0x0800,
};

struct SquashSuperblock;

int squash_data_superblock_init(
		const struct SquashSuperblock *superblock, size_t size);

uint32_t
squash_data_superblock_magic(const struct SquashSuperblock *superblock);
uint32_t
squash_data_superblock_inode_count(const struct SquashSuperblock *superblock);
uint32_t squash_data_superblock_modification_time(
		const struct SquashSuperblock *superblock);
uint32_t
squash_data_superblock_block_size(const struct SquashSuperblock *superblock);
uint32_t squash_data_superblock_fragment_entry_count(
		const struct SquashSuperblock *superblock);
uint16_t squash_data_superblock_compression_id(
		const struct SquashSuperblock *superblock);
uint16_t
squash_data_superblock_block_log(const struct SquashSuperblock *superblock);
uint16_t
squash_data_superblock_flags(const struct SquashSuperblock *superblock);
uint16_t
squash_data_superblock_id_count(const struct SquashSuperblock *superblock);
uint16_t
squash_data_superblock_version_major(const struct SquashSuperblock *superblock);
uint16_t
squash_data_superblock_version_minor(const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_root_inode_ref(
		const struct SquashSuperblock *superblock);
uint64_t
squash_data_superblock_bytes_used(const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_id_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_xattr_id_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_inode_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_directory_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_fragment_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_data_superblock_export_table_start(
		const struct SquashSuperblock *superblock);

#endif /* end of include guard SQUASH_SUPERBLOCK_H */
