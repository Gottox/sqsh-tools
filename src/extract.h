/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock2
 * @created     : Saturday Sep 04, 2021 23:13:19 CEST
 */

#include "extractor/extractor.h"
#include "format/metablock.h"
#include <stdint.h>

#ifndef EXTRACT_H

#define EXTRACT_H

struct Squash;

struct SquashExtract {
	const struct SquashExtractor *extractor;
	const struct SquashMetablock *start_block;
	off_t index;
	off_t offset;
	uint8_t *extracted;
	size_t extracted_size;
};

int squash_extract_init(struct SquashExtract *extract,
		const struct Squash *squash, const struct SquashMetablock *block,
		off_t block_index, off_t block_offset);
int squash_extract_more(struct SquashExtract *extract, const size_t size);
void *squash_extract_data(const struct SquashExtract *extract);
size_t squash_extract_size(const struct SquashExtract *extract);
int squash_extract_cleanup(struct SquashExtract *extract);

#endif /* end of include guard EXTRACT_H */
