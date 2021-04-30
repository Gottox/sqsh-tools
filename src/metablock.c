/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:39:44 CEST
 */

#include "metablock.h"

#include <assert.h>
#include <stdint.h>

struct SquashMetablock *
squash_metablock_new(struct Squash *squash, off_t offset) {
	struct SquashMetablock *metablock;

	metablock = calloc(1, sizeof(struct SquashMetablock));
	if (metablock == NULL) {
		return NULL;
	}
	metablock->root = squash;
	metablock->wrap =
		(struct SquashMetablockWrap *)&((uint8_t *)squash->superblock)[offset];

	return metablock;
}

int
squash_metablock_is_compressed(struct SquashMetablock *metablock) {
	return !(metablock->wrap->header & 0x8000);
}

uint8_t *
squash_metablock_data(struct SquashMetablock *metablock) {
	if (metablock->data) {
		return metablock->data;
	} else if (squash_metablock_is_compressed(metablock)) {
		assert(0 == "TODO");
		return NULL;
	} else {
		return metablock->data = metablock->wrap->data;
	}
}

size_t
squash_metablock_size(struct SquashMetablock *metablock) {
	if (metablock->size) {
		return metablock->size;
	} else if (squash_metablock_is_compressed(metablock)) {
		assert(0 == "TODO");
		return 0;
	} else {
		return metablock->size = metablock->wrap->header & 0x7FFF;
	}
}

int
squash_metablock_cleanup(struct SquashMetablock *metablock) {
	if (metablock == NULL) {
		return 0;
	}

	if (squash_metablock_is_compressed(metablock)) {
		free(metablock->wrap->data);
	}
	free(metablock);

	return 0;
}
