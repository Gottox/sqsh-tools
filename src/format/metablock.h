/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:22:31 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#ifndef METABLOCK_H

#define METABLOCK_H

struct Squash;

struct SquashMetablock {
	uint16_t header;
	uint8_t data[0];
};

const struct SquashMetablock *squash_metablock_from_offset(
		struct Squash *squash, off_t offset);

const struct SquashMetablock *squash_metablock_from_start_block(
		const struct SquashMetablock *start_block, off_t offset);

int squash_metablock_is_compressed(const struct SquashMetablock *metablock);

int squash_metablock_is_empty(const struct SquashMetablock *metablock);

const uint8_t *squash_metablock_data(const struct SquashMetablock *metablock);

size_t squash_metablock_size(const struct SquashMetablock *metablock);

#endif /* end of include guard METABLOCK_H */
