/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock_context
 * @created     : Monday Oct 11, 2021 13:41:59 CEST
 */

#include "../utils.h"

#ifndef SUPERBLOCK_CONTEXT_H

#define SUPERBLOCK_CONTEXT_H

struct SquashSuperblock;

struct SquashSuperblockContext {
	const struct SquashSuperblock *superblock;
};

SQUASH_NO_UNUSED int squash_superblock_init(
		struct SquashSuperblockContext *context, const uint8_t *buffer,
		size_t size);

int squash_superblock_cleanup(struct SquashSuperblockContext *superblock);

#endif /* end of include guard SUPERBLOCK_CONTEXT_H */
