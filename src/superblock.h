/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock
 * @created     : Friday Apr 30, 2021 12:36:57 CEST
 */

#ifndef SUPERBLOCK_H

#define SUPERBLOCK_H

#include <stdint.h>
#include <stdlib.h>
#define SQUASH_SUPERBLOCK_SIZE (sizeof(struct SquashSuperblockWrap))
#define SQUASH_SUPERBLOCK_MAGIC 0x73717368

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

struct SquashSuperblockWrap {
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

struct SquashSuperblock {
	struct SquashSuperblockWrap *wrap;
};

int squash_superblock_init(
		struct SquashSuperblock *superblock, uint8_t *bytes, size_t size);

int squash_superblock_cleanup(struct SquashSuperblockWrap *superblock);

#endif /* end of include guard SUPERBLOCK_H */
