/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:28:28 CEST
 */

#include "../data/fragment.h"
#include "../data/superblock.h"
#include "metablock_context.h"
#include <stdint.h>

#ifndef FRAGMENT_CONTEXT_H

#define FRAGMENT_CONTEXT_H

struct SquashFragmentContext {
	const struct SquashSuperblock *superblock;
	const struct SquashMetablock *table;
	struct SquashMetablockContext extract;
	uint32_t fragment_count;
};

int squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblock *superblock);

const uint32_t *squash_fragment_data(
		struct SquashFragmentContext *fragment, uint32_t index);

uint32_t squash_fragment_count(struct SquashFragmentContext *fragment);

#endif /* end of include guard FRAGMENT_CONTEXT_H */
