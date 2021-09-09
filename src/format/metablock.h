/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : metablock
 * @created     : Wednesday Sep 08, 2021 14:27:47 CEST
 */

#include <stddef.h>
#include <stdint.h>

#ifndef SQUASH_METABLOCK_H

#define SQUASH_METABLOCK_H

struct SquashMetablock;

int squash_format_metablock_is_compressed(
		const struct SquashMetablock *metablock);

const uint8_t *squash_format_metablock_data(
		const struct SquashMetablock *metablock);

size_t squash_format_metablock_size(const struct SquashMetablock *metablock);

#endif /* end of include guard METABLOCK_H */
