/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : utils
 * @created     : Friday Apr 30, 2021 12:35:31 CEST
 */

#include <stdint.h>

#ifndef SQUASH_UTILS_H

#define SQUASH_UTILS_H

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define ADD_OVERFLOW(a, b, res) __builtin_add_overflow(a, b, res)

uint32_t log2_u32(uint32_t x);

#endif /* end of include guard SQUASH_UTILS_H */
