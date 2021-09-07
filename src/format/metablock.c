/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock.h"
#include "../squash.h"
#include "../utils.h"
#include "superblock.h"

#include <assert.h>
#include <stdint.h>

struct SquashMetablock {
	uint16_t header;
	// uint8_t data[0];
};
CASSERT(sizeof(struct SquashMetablock) == SQUASH_METABLOCK_SIZE);

const struct SquashMetablock *
squash_metablock_from_offset(struct Squash *squash, off_t offset) {
	const uint8_t *tmp = (uint8_t *)squash->superblock;

	if (offset >= squash->size) {
		return NULL;
	} else {
		return (const struct SquashMetablock *)&tmp[offset];
	}
}

const struct SquashMetablock *
squash_metablock_from_start_block(
		const struct SquashMetablock *start_block, off_t offset) {
	const uint8_t *tmp = (uint8_t *)start_block;

	// TODO overflow
	return (const struct SquashMetablock *)&tmp[offset];
}

int
squash_metablock_is_compressed(const struct SquashMetablock *metablock) {
	return !(metablock->header & 0x8000);
}

const uint8_t *
squash_metablock_data(const struct SquashMetablock *metablock) {
	const uint8_t *tmp = (uint8_t *)metablock;
	return (uint8_t *)&tmp[sizeof(struct SquashMetablock)];
}

size_t
squash_metablock_size(const struct SquashMetablock *metablock) {
	return metablock->header & 0x7FFF;
}
