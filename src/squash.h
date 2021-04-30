/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : squash
 * @created     : Friday Apr 30, 2021 10:58:14 CEST
 */

#ifndef SQUASH_H

#define SQUASH_H

#include <stdint.h>
#include <stdlib.h>

#include "superblock.h"
#include "metablock.h"
#include "compression/compression.h"

#define SQUASH_SUPERBLOCK_SIZE (sizeof(struct SquashSuperblockWrap))
#define SQUASH_SUPERBLOCK_MAGIC 0x73717368

enum SquashError {
	// Avoid collisions with errno
	SQUASH_SUCCESS = 1 << 8,
	SQUASH_ERROR_CHECKFLAG_SET,
};

enum SquashDtor {
	SQUASH_DTOR_NONE,
	SQUASH_DTOR_FREE,
	SQUASH_DTOR_MUNMAP,
};

struct Squash {
	uint32_t error;
	struct SquashSuperblockWrap *superblock;
	int size;
	enum SquashDtor dtor;
	struct SquashMetablock *compression_options;
};

struct Squash *squash_init(uint8_t *buffer, const size_t size, const enum SquashDtor dtor);
struct Squash *squash_open(const char *path);
int squash_cleanup(struct Squash *squash);

#endif /* end of include guard SQUASH_H */
