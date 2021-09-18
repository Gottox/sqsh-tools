/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock_context
 * @created     : Saturday Sep 04, 2021 23:15:47 CEST
 */

#include "metablock_context.h"
#include "../compression/compression.h"
#include "../data/metablock_internal.h"
#include "../data/superblock.h"
#include "../error.h"

int
squash_extract_init(struct SquashMetablockContext *extract,
		const struct SquashSuperblock *superblock,
		const struct SquashMetablock *block, off_t block_index,
		off_t block_offset) {
	int rv = 0;
	extract->extracted = NULL;
	extract->extracted_size = 0;
	extract->start_block = block;
	extract->index = block_index;
	extract->offset = block_offset;
	extract->superblock = superblock;
	if (squash_data_metablock_is_compressed(block)) {
		rv = squash_compression_init(&extract->compression, superblock);
	} else {
		extract->compression.impl = &squash_compression_null;
		extract->compression.options = NULL;
	}
	return rv;
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
		const size_t block_size = squash_data_metablock_size(block);
		if (block_size == 0) {
			return -SQUASH_ERROR_METABLOCK_ZERO_SIZE;
		}

		const void *block_data = squash_data_metablock_data(block);
		rv = squash_compression_extract(&extract->compression,
				&extract->extracted, &extract->extracted_size, block_data,
				block_size);
		extract->index += sizeof(struct SquashMetablock) + block_size;
	}
	return rv;
}

void *
squash_extract_data(const struct SquashMetablockContext *extract) {
	return &extract->extracted[extract->offset];
}

size_t
squash_extract_size(const struct SquashMetablockContext *extract) {
	if (extract->offset > extract->extracted_size) {
		return 0;
	} else {
		return extract->extracted_size - extract->offset;
	}
}

int
squash_extract_cleanup(struct SquashMetablockContext *extract) {
	free(extract->extracted);
	return squash_compression_cleanup(&extract->compression);
}
