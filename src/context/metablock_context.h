/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock2
 * @created     : Saturday Sep 04, 2021 23:13:19 CEST
 */

#include "../compression/compression.h"
#include "../data/metablock.h"
#include "../utils.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef SQUASH_EXTRACT_H

#define SQUASH_EXTRACT_H

struct SquashMetablockContext {
	const struct SquashSuperblock *superblock;
	struct SquashCompression compression;
	const struct SquashMetablock *start_block;
	off_t index;
	off_t offset;
	uint8_t *extracted;
	size_t extracted_size;
};

SQUASH_NO_UNUSED const struct SquashMetablock *squash_metablock_from_offset(
		const struct SquashSuperblock *superblock, off_t offset);

SQUASH_NO_UNUSED const struct SquashMetablock *
squash_metablock_from_start_block(const struct SquashSuperblock *superblock,
		const struct SquashMetablock *start_block, off_t offset);

SQUASH_NO_UNUSED int squash_extract_init(struct SquashMetablockContext *extract,
		const struct SquashSuperblock *superblock,
		const struct SquashMetablock *block, off_t block_index,
		off_t block_offset);
SQUASH_NO_UNUSED int squash_extract_more(
		struct SquashMetablockContext *extract, const size_t size);
SQUASH_NO_UNUSED void *squash_extract_data(
		const struct SquashMetablockContext *extract);
SQUASH_NO_UNUSED size_t squash_extract_size(
		const struct SquashMetablockContext *extract);
int squash_extract_cleanup(struct SquashMetablockContext *extract);

#endif /* end of include guard SQUASH_EXTRACT_H */
