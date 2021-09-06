/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : extractor
 * @created     : Saturday Sep 04, 2021 23:15:47 CEST
 */

#include "extract.h"
#include "compression/compression.h"
#include "squash.h"

const struct SquashExtractor static_null_extractor = {
		.impl = &squash_extractor_null, .options = NULL};

int
squash_extract_init(struct SquashExtract *extract, const struct Squash *squash,
		const struct SquashMetablock *block, off_t block_index,
		off_t block_offset) {
	extract->extracted = NULL;
	extract->extracted_size = 0;
	extract->start_block = block;
	extract->index = block_index;
	extract->offset = block_offset;
	if (squash_metablock_is_compressed(block)) {
		extract->extractor = &squash->extractor;
	} else {
		extract->extractor = &static_null_extractor;
	}
	return 0;
}

int
squash_extract_more(struct SquashExtract *extract, const size_t size) {
	int rv = 0;
	const struct SquashExtractorImplementation *impl = extract->extractor->impl;
	const union SquashCompressionOptions *options = extract->extractor->options;
	const struct SquashMetablock *start_block = extract->start_block;

	for (; rv == 0 && size > squash_extract_size(extract);) {
		const struct SquashMetablock *block =
				squash_metablock_from_start_block(start_block, extract->index);
		const size_t block_size = squash_metablock_size(block);
		rv = impl->extract(options, &extract->extracted,
				&extract->extracted_size, squash_metablock_data(block),
				block_size);
		extract->index += block_size + sizeof(struct SquashMetablock);
	}
	return 0;
}

void *
squash_extract_data(const struct SquashExtract *extract) {
	return &extract->extracted[extract->offset];
}

size_t
squash_extract_size(const struct SquashExtract *extract) {
	if (extract->offset > extract->extracted_size) {
		return 0;
	} else {
		return extract->extracted_size - extract->offset;
	}
}

int
squash_extract_cleanup(struct SquashExtract *extract) {
	free(extract->extracted);
	return 0;
}
