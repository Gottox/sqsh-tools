/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock2
 * @created     : Saturday Sep 04, 2021 23:13:19 CEST
 */

#include "../compression/buffer.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef SQUASH_EXTRACT_H

#define SQUASH_EXTRACT_H

struct SquashMetablockContext {
	const struct SquashSuperblock *superblock;
	struct SquashBuffer buffer;
	off_t start_block;
	off_t index;
	off_t offset;
};

// DEPRECATED:
const struct SquashMetablock *squash_metablock_from_offset(
		const struct SquashSuperblock *superblock, off_t offset);
SQUASH_NO_UNUSED int squash_metablock_init(
		struct SquashMetablockContext *extract,
		const struct SquashSuperblock *superblock, off_t start_block);
SQUASH_NO_UNUSED int squash_metablock_seek(
		struct SquashMetablockContext *metablock, off_t index, off_t offset);
SQUASH_NO_UNUSED int squash_metablock_more(
		struct SquashMetablockContext *metablock, const size_t size);
const uint8_t *squash_metablock_data(
		const struct SquashMetablockContext *metablock);
size_t squash_metablock_size(const struct SquashMetablockContext *metablock);
int squash_metablock_cleanup(struct SquashMetablockContext *metablock);

#endif /* end of include guard SQUASH_EXTRACT_H */
