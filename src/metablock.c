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

int
squash_metablock_init(struct SquashMetablock* metablock, struct Squash *squash, off_t offset) {
	metablock->decompressor = &squash->decompressor;
	metablock->wrap =
		(struct SquashMetablockWrap *)&((uint8_t *)squash->superblock)[offset];

	return 0;
}

int
squash_metablock_is_compressed(struct SquashMetablock *metablock) {
	return !(metablock->wrap->header & 0x8000);
}

static int
uncompress(struct SquashMetablock *metablock) {
	int compressed_size = squash_metablock_compressed_size(metablock);
	struct SquashDecompressor *de = metablock->decompressor;

	de->impl->decompress(
		&de->info, &metablock->data, &metablock->size, metablock->wrap->data, 0,
		compressed_size);

	return 0;
}

uint8_t *
squash_metablock_data(struct SquashMetablock *metablock) {
	if (metablock->data) {
		return metablock->data;
	} else if (squash_metablock_is_compressed(metablock)) {
		if (uncompress(metablock) < 0) {
			return NULL;
		}
	} else {
		metablock->data = metablock->wrap->data;
	}
	return metablock->data;
}

size_t
squash_metablock_compressed_size(struct SquashMetablock *metablock) {
	return metablock->wrap->header & 0x7FFF;
}

size_t
squash_metablock_size(struct SquashMetablock *metablock) {
	if (metablock->size) {
		return metablock->size;
	} else if (squash_metablock_is_compressed(metablock)) {
		assert(0 == "TODO");
		return 0;
	} else {
		return metablock->size = squash_metablock_compressed_size(metablock);
	}
}

int
squash_metablock_cleanup(struct SquashMetablock *metablock) {
	if (squash_metablock_is_compressed(metablock)) {
		free(metablock->wrap->data);
	}

	return 0;
}
