/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock
 * @created     : Friday Apr 30, 2021 12:36:57 CEST
 */

#ifndef SQUASH__SUPERBLOCK_H

#define SQUASH__SUPERBLOCK_H

#include <stdint.h>
#include <stdlib.h>

#define SQUASH_SUPERBLOCK_SIZE 96

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

int squash_superblock_init(
		const struct SquashSuperblock *superblock, size_t size);

uint32_t squash_superblock_magic(const struct SquashSuperblock *superblock);
uint32_t squash_superblock_inode_count(
		const struct SquashSuperblock *superblock);
uint32_t squash_superblock_modification_time(
		const struct SquashSuperblock *superblock);
uint32_t squash_superblock_block_size(
		const struct SquashSuperblock *superblock);
uint32_t squash_superblock_fragment_entry_count(
		const struct SquashSuperblock *superblock);
uint16_t squash_superblock_compression_id(
		const struct SquashSuperblock *superblock);
uint16_t squash_superblock_block_log(const struct SquashSuperblock *superblock);
uint16_t squash_superblock_flags(const struct SquashSuperblock *superblock);
uint16_t squash_superblock_id_count(const struct SquashSuperblock *superblock);
uint16_t squash_superblock_version_major(
		const struct SquashSuperblock *superblock);
uint16_t squash_superblock_version_minor(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_root_inode_ref(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_bytes_used(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_id_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_xattr_id_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_inode_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_directory_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_fragment_table_start(
		const struct SquashSuperblock *superblock);
uint64_t squash_superblock_export_table_start(
		const struct SquashSuperblock *superblock);

#endif /* end of include guard SQUASH__SUPERBLOCK_H */
