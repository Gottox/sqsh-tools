/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock_context
 * @created     : Saturday Sep 04, 2021 23:15:47 CEST
 */

#include "metablock_context.h"
#include "../compression/buffer.h"
#include "../data/metablock_internal.h"
#include "../data/superblock.h"
#include "../error.h"
#include <stdint.h>

#define BLOCK_SIZE 8192

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

int
squash_metablock_init(struct SquashMetablockContext *extract,
		const struct SquashSuperblock *superblock, off_t start_block) {
	extract->start_block = start_block;
	extract->index = 0;
	extract->offset = 0;
	extract->superblock = superblock;
	return squash_metablock_seek(extract, 0, 0);
}

int
squash_metablock_seek(struct SquashMetablockContext *metablock,
		const off_t index, const off_t offset) {
	int rv = 0;

	squash_buffer_cleanup(&metablock->buffer);

	metablock->index = index;
	metablock->offset = offset;
	squash_buffer_cleanup(&metablock->buffer);
	rv = squash_buffer_init(
			&metablock->buffer, metablock->superblock, BLOCK_SIZE);

	return rv;
}

int
squash_metablock_more(
		struct SquashMetablockContext *extract, const size_t size) {
	int rv = 0;
	const struct SquashMetablock *start_block = squash_metablock_from_offset(
			extract->superblock, extract->start_block);

	if (start_block == NULL) {
		return -SQUASH_ERROR_TODO;
	}

	for (; rv == 0 && size > squash_metablock_size(extract);) {
		const struct SquashMetablock *block = squash_metablock_from_start_block(
				extract->superblock, start_block, extract->index);
		if (block == NULL) {
			return -SQUASH_ERROR_TODO;
		}
		const size_t metablock_size = squash_data_metablock_size(block);
		if (metablock_size == 0) {
			return -SQUASH_ERROR_METABLOCK_ZERO_SIZE;
		}

		const void *block_data = squash_data_metablock_data(block);
		bool is_compressed = squash_data_metablock_is_compressed(block);
		rv = squash_buffer_append(
				&extract->buffer, block_data, metablock_size, is_compressed);
		extract->index += sizeof(struct SquashMetablock) + metablock_size;
	}
	return rv;
}

const uint8_t *
squash_metablock_data(const struct SquashMetablockContext *extract) {
	return &extract->buffer.data[extract->offset];
}

size_t
squash_metablock_size(const struct SquashMetablockContext *extract) {
	const size_t buffer_size = squash_buffer_size(&extract->buffer);
	if (extract->offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - extract->offset;
	}
}

int
squash_metablock_cleanup(struct SquashMetablockContext *extract) {
	return squash_buffer_cleanup(&extract->buffer);
}
