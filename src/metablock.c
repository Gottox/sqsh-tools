/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock.h"
#include "compression/compression.h"
#include "squash.h"
#include "superblock.h"

#include <assert.h>
#include <stdint.h>

extern struct SquashDecompressorImpl squash_null_deflate;

static struct SquashDecompressor null_decompressor = {
		.info = {0},
		.impl = &squash_null_deflate,
};

int
squash_metablock_init(struct SquashMetablock *metablock, struct Squash *squash,
		off_t offset) {
	uint8_t *dumb_ptr = (uint8_t *)squash->superblock.wrap;

	// sanity-checks
	if (offset < SQUASH_SUPERBLOCK_SIZE)
		return -SQUASH_ERROR_METABLOCK_INIT;

	metablock->wrap = (struct SquashMetablockWrap *)(dumb_ptr + offset);

	if (squash_metablock_is_compressed(metablock)) {
		metablock->decompressor = &squash->decompressor;
	} else {
		metablock->decompressor = &null_decompressor;
	}

	return 0;
}

int
squash_metablock_is_empty(struct SquashMetablock *metablock) {
	return metablock == NULL || metablock->wrap == NULL;
}

int
squash_metablock_is_compressed(struct SquashMetablock *metablock) {
	return !(metablock->wrap->header & 0x8000);
}

uint8_t *
squash_metablock_data(struct SquashMetablock *metablock) {
	return metablock->wrap->data;
}

size_t
squash_metablock_size(struct SquashMetablock *metablock) {
	return metablock->wrap->header & 0x7FFF;
}

int
squash_metablock_cleanup(struct SquashMetablock *metablock) {
	if (squash_metablock_is_compressed(metablock)) {
		free(metablock->wrap->data);
	}

	return 0;
}
