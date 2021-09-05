/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock.h"
#include "../squash.h"
#include "../superblock.h"

#include <assert.h>
#include <stdint.h>

const struct SquashMetablock *
squash_metablock_from_offset(struct Squash *squash, off_t offset) {
	void *tmp = squash->superblock.wrap;

	if (offset >= squash->size) {
		return NULL;
	} else {
		return tmp + offset;
	}
}

const struct SquashMetablock *
squash_metablock_from_start_block(const struct SquashMetablock *start_block, off_t offset) {
	const void *tmp = start_block;

	// TODO overflow
	return tmp + offset;
}


int
squash_metablock_is_compressed(const struct SquashMetablock *metablock) {
	return !(metablock->header & 0x8000);
}

const uint8_t *
squash_metablock_data(const struct SquashMetablock *metablock) {
	return metablock->data;
}

size_t
squash_metablock_size(const struct SquashMetablock *metablock) {
	return metablock->header & 0x7FFF;
}
