/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment
 * @created     : Friday May 07, 2021 06:59:17 CEST
 */

#include <stdint.h>

#ifndef SQUASH__FRAGMENT_H

#define SQUASH__FRAGMENT_H

#define SQUASH_SIZEOF_FRAGMENT 16

struct SquashFragment;

uint64_t squash_data_fragment_start(const struct SquashFragment *fragment);
uint32_t squash_data_fragment_size(const struct SquashFragment *fragment);

#endif /* end of include guard SQUASH__FRAGMENT_H */
