/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Friday Apr 30, 2021 21:22:31 CEST
 */

#include "../squash.h"
#include <stdlib.h>

#ifndef SQUASH_METABLOCK_CONTEXT_H

#define SQUASH_METABLOCK_CONTEXT_H

struct SquashMetablock;

const struct SquashMetablock *squash_metablock_from_offset(
		struct Squash *squash, off_t offset);

const struct SquashMetablock *squash_metablock_from_start_block(
		const struct SquashMetablock *start_block, off_t offset);

#endif /* end of include guard SQUASH_FORMAT_METABLOCK_H */