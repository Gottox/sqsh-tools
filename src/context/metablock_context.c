/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock_context.h"
#include "../data/metablock.h"

#include "../data/superblock.h"
#include "../squash.h"
#include "../utils.h"
#include <stdint.h>

static int
metablock_bounds_check(const struct SquashSuperblock *superblock,
		const struct SquashMetablock *block) {
	uint64_t upper_bounds;
	uint64_t header_bounds;
	uint64_t data_bounds;

	if (ADD_OVERFLOW((uint64_t)superblock,
				squash_data_superblock_bytes_used(superblock), &upper_bounds)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}

	if (ADD_OVERFLOW(
				(uint64_t)block, SQUASH_SIZEOF_METABLOCK, &header_bounds)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}

	if (header_bounds > upper_bounds) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	const uint8_t *data = squash_data_metablock_data(block);
	const size_t data_size = squash_data_metablock_size(block);
	if (ADD_OVERFLOW((uint64_t)data, data_size, &data_bounds)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}

	if (data_bounds > upper_bounds) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	return 0;
}

const struct SquashMetablock *
squash_metablock_from_offset(
		const struct SquashSuperblock *superblock, off_t offset) {
	const uint8_t *tmp = (uint8_t *)superblock;
	const struct SquashMetablock *block =
			(const struct SquashMetablock *)&tmp[offset];
	if (metablock_bounds_check(superblock, block) < 0) {
		return NULL;
	} else {
		return block;
	}
}

const struct SquashMetablock *
squash_metablock_from_start_block(const struct SquashSuperblock *superblock,
		const struct SquashMetablock *start_block, off_t offset) {
	const uint8_t *tmp = (uint8_t *)start_block;
	const struct SquashMetablock *block =
			(const struct SquashMetablock *)&tmp[offset];

	if (metablock_bounds_check(superblock, block) < 0) {
		return NULL;
	} else {
		return block;
	}
}
