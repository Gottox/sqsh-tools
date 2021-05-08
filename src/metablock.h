/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:22:31 CEST
 */

#ifndef METABLOCK_H

#define METABLOCK_H

#include <stdint.h>
#include <stdlib.h>

struct Squash;

struct SquashDecompressor;

struct SquashMetablockWrap {
	uint16_t header;
	uint8_t data[0];
};

struct SquashMetablock {
	struct SquashMetablockWrap *wrap;
	struct SquashDecompressor *decompressor;
	uint8_t *data;
	size_t size;
};

int squash_metablock_init(
		struct SquashMetablock *metablock, struct Squash *squash, off_t offset);

int squash_metablock_is_compressed(struct SquashMetablock *metablock);

uint8_t *squash_metablock_data(struct SquashMetablock *metablock);

size_t squash_metablock_size(struct SquashMetablock *metablock);

size_t squash_metablock_compressed_size(struct SquashMetablock *metablock);

int squash_metablock_cleanup(struct SquashMetablock *metablock);

#endif /* end of include guard METABLOCK_H */
