/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock_context
 * @created     : Monday Oct 11, 2021 13:41:47 CEST
 */

#include "superblock_context.h"
#include "../data/superblock.h"
#include "../error.h"
#include <stdint.h>

const static uint32_t SUPERBLOCK_MAGIC = 0x73717368;

int
squash_superblock_init(struct SquashSuperblockContext *context,
		const uint8_t *buffer, size_t size) {
	const struct SquashSuperblock *superblock =
			(const struct SquashSuperblock *)buffer;
	if (size < SQUASH_SIZEOF_SUPERBLOCK) {
		return -SQUASH_ERROR_SUPERBLOCK_TOO_SMALL;
	}

	// Do not use the getter here as it may change the endianess. We don't want
	// that here.
	if (squash_data_superblock_magic(superblock) != SUPERBLOCK_MAGIC) {
		return -SQUASH_ERROR_WRONG_MAGIG;
	}

	if (squash_data_superblock_block_log(superblock) !=
			squash_log2_u32(squash_data_superblock_block_size(superblock))) {
		return -SQUASH_ERROR_BLOCKSIZE_MISSMATCH;
	}

	if (squash_data_superblock_bytes_used(superblock) > size) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	context->superblock = superblock;

	return 0;
}

int
squash_superblock_cleanup(struct SquashSuperblockContext *superblock) {
	return 0;
}
