/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock_context.h"
#include "../format/superblock.h"
#include "../squash.h"
#include "../utils.h"

#include <assert.h>
#include <stdint.h>

struct SquashMetablock {
	uint16_t header;
	// uint8_t data[0];
};

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
