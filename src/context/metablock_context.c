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

#define BLOCK_SIZE 8192

int
squash_extract_init(struct SquashMetablockContext *extract,
		const struct SquashSuperblock *superblock,
		const struct SquashMetablock *block, off_t block_index,
		off_t block_offset) {
	extract->start_block = block;
	extract->index = block_index;
	extract->offset = block_offset;
	extract->superblock = superblock;
	return squash_buffer_init(&extract->buffer, superblock, BLOCK_SIZE);
}

int
squash_extract_more(struct SquashMetablockContext *extract, const size_t size) {
	int rv = 0;
	const struct SquashMetablock *start_block = extract->start_block;

	for (; rv == 0 && size > squash_extract_size(extract);) {
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

void *
squash_extract_data(const struct SquashMetablockContext *extract) {
	return &extract->buffer.data[extract->offset];
}

size_t
squash_extract_size(const struct SquashMetablockContext *extract) {
	const size_t buffer_size = squash_buffer_size(&extract->buffer);
	if (extract->offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - extract->offset;
	}
}

int
squash_extract_cleanup(struct SquashMetablockContext *extract) {
	return squash_buffer_cleanup(&extract->buffer);
}
