/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : utils
 * @created     : Friday Apr 30, 2021 12:35:31 CEST
 */

#include <stddef.h>
#include <stdint.h>

#ifndef SQUASH_UTILS_H

#define SQUASH_UTILS_H

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define ADD_OVERFLOW(a, b, res) __builtin_add_overflow(a, b, res)

#define SQUASH_NO_UNUSED __attribute__((warn_unused_result))

#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

uint32_t squash_log2_u32(uint32_t x);

uint32_t squash_divide_ceil_u32(uint32_t x, uint32_t y);

SQUASH_NO_UNUSED int squash_memdup(
		char **target, const char *source, size_t size);

#endif /* end of include guard SQUASH_UTILS_H */
