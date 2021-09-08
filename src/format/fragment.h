/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment
 * @created     : Friday May 07, 2021 06:59:17 CEST
 */

#include <stdint.h>

#ifndef SQUASH_FORMAT_FRAGMENT_H

#define SQUASH_FORMAT_FRAGMENT_H

struct SquashFragment {
	uint64_t start;
	uint32_t size;
	uint32_t unused;
};

#endif /* end of include guard SQUASH_FORMAT_FRAGMENT_H */
