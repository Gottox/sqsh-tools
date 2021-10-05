/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:28:28 CEST
 */

#include "table_context.h"
#include <stdint.h>

#ifndef FRAGMENT_CONTEXT_H

#define FRAGMENT_CONTEXT_H

struct SquashInodeContext;

struct SquashFragmentContext {
	const struct SquashSuperblock *superblock;
	const struct SquashInodeContext *inode;
	// TODO: This table should be part of struct Squash.
	struct SquashTableContext table;
	const struct SquashFragment *fragment;
	struct SquashBuffer buffer;
};

int squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode);

uint64_t squash_fragment_start(struct SquashFragmentContext *fragment);

uint32_t squash_fragment_size(struct SquashFragmentContext *fragment);

const uint8_t *squash_fragment_data(struct SquashFragmentContext *fragment);

int squash_fragment_clean(struct SquashFragmentContext *fragment);

#endif /* end of include guard FRAGMENT_CONTEXT_H */
