/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock
 * @created     : Friday Apr 30, 2021 12:34:53 CEST
 */

#include "superblock.h"
#include "../error.h"
#include "../utils.h"

#include <endian.h>
#include <stdint.h>

static const union {
	char c[4];
	uint32_t d;
} superblock_magic = {.c = "hsqs"};

struct SquashSuperblock {
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
CASSERT(sizeof(struct SquashSuperblock) == SQUASH_SUPERBLOCK_SIZE);

int
squash_superblock_init(const struct SquashSuperblock *superblock, size_t size) {
	if (size < sizeof(struct SquashSuperblock)) {
		return -SQUASH_ERROR_SUPERBLOCK_TOO_SMALL;
	}

	// Do not use the getter here as it may change the endianess. We don't want
	// that here.
	if (superblock->magic != superblock_magic.d) {
		return -SQUASH_ERROR_WRONG_MAGIG;
	}
	if (squash_superblock_block_log(superblock) !=
			log2_u32(squash_superblock_block_size(superblock))) {
		return -SQUASH_ERROR_BLOCKSIZE_MISSMATCH;
	}

	return 0;
}

uint32_t
squash_superblock_magic(const struct SquashSuperblock *superblock) {
	return le32toh(superblock->magic);
}

uint32_t
squash_superblock_inode_count(const struct SquashSuperblock *superblock) {
	return le32toh(superblock->inode_count);
}

uint32_t
squash_superblock_modification_time(const struct SquashSuperblock *superblock) {
	return le32toh(superblock->modification_time);
}

uint32_t
squash_superblock_block_size(const struct SquashSuperblock *superblock) {
	return le32toh(superblock->block_size);
}

uint32_t
squash_superblock_fragment_entry_count(
		const struct SquashSuperblock *superblock) {
	return le32toh(superblock->fragment_entry_count);
}

uint16_t
squash_superblock_compression_id(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->compression_id);
}

uint16_t
squash_superblock_block_log(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->block_log);
}

uint16_t
squash_superblock_flags(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->flags);
}

uint16_t
squash_superblock_id_count(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->id_count);
}

uint16_t
squash_superblock_version_major(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->version_major);
}

uint16_t
squash_superblock_version_minor(const struct SquashSuperblock *superblock) {
	return le16toh(superblock->version_minor);
}

uint64_t
squash_superblock_root_inode_ref(const struct SquashSuperblock *superblock) {
	return le64toh(superblock->root_inode_ref);
}

uint64_t
squash_superblock_bytes_used(const struct SquashSuperblock *superblock) {
	return le64toh(superblock->bytes_used);
}

uint64_t
squash_superblock_id_table_start(const struct SquashSuperblock *superblock) {
	return le64toh(superblock->id_table_start);
}

uint64_t
squash_superblock_xattr_id_table_start(
		const struct SquashSuperblock *superblock) {
	return le64toh(superblock->xattr_id_table_start);
}

uint64_t
squash_superblock_inode_table_start(const struct SquashSuperblock *superblock) {
	return le64toh(superblock->inode_table_start);
}

uint64_t
squash_superblock_directory_table_start(
		const struct SquashSuperblock *superblock) {
	return le64toh(superblock->directory_table_start);
}

uint64_t
squash_superblock_fragment_table_start(
		const struct SquashSuperblock *superblock) {
	return le64toh(superblock->fragment_table_start);
}

uint64_t
squash_superblock_export_table_start(
		const struct SquashSuperblock *superblock) {
	return le64toh(superblock->export_table_start);
}
