/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : squash
 * @created     : Friday Apr 30, 2021 10:58:14 CEST
 */

#include "compression/compression.h"
#include "error.h"
#include "format/superblock.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef SQUASH_H

#define SQUASH_H

enum SquashDtor {
	SQUASH_DTOR_NONE,
	SQUASH_DTOR_FREE,
	SQUASH_DTOR_MUNMAP,
};

struct Squash {
	uint32_t error;
	const struct SquashSuperblock *superblock;
	int size;
	uint8_t *buffer;
	enum SquashDtor dtor;
	struct SquashExtractor extractor;
};

int squash_init(struct Squash *squash, uint8_t *buffer, const size_t size,
		const enum SquashDtor dtor);

int squash_open(struct Squash *squash, const char *path);

int squash_cleanup(struct Squash *squash);

#endif /* end of include guard SQUASH_H */
