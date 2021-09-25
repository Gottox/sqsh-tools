/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_internal
 * @created     : Friday Sep 17, 2021 09:29:00 CEST
 */

#include "../utils.h"
#include "datablock_internal.h"
#include "fragment.h"

#ifndef FRAGMENT_INTERNAL_H

#define FRAGMENT_INTERNAL_H

struct SquashFragment {
	uint64_t start;
	struct SquashDatablockSize size;
	uint32_t unused;
};

STATIC_ASSERT(sizeof(struct SquashFragment) == SQUASH_SIZEOF_FRAGMENT);

#endif /* end of include guard FRAGMENT_INTERNAL_H */
