/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock_context.h"
#include "../format/metablock_internal.h"

#include "../format/superblock.h"
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
				squash_superblock_bytes_used(superblock), &upper_bounds)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}

	if (ADD_OVERFLOW((uint64_t)block, sizeof(struct SquashMetablock),
				&header_bounds)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}

	if (header_bounds > upper_bounds) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	if (ADD_OVERFLOW((uint64_t)squash_format_metablock_data(block),
				squash_format_metablock_size(block), &data_bounds)) {
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
