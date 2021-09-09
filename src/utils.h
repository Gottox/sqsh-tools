/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : utils
 * @created     : Friday Apr 30, 2021 12:35:31 CEST
 */

#include <stdint.h>

#ifndef SQUASH_UTILS_H

#define SQUASH_UTILS_H

#define ADD_OVERFLOW(a, b, res) __builtin_add_overflow(a, b, res)

#define CASSERT(predicate) _impl_CASSERT_LINE(predicate, __LINE__)
#define _impl_PASTE(a, b) a##b
#define _impl_CASSERT_LINE(predicate, line) \
	typedef char _impl_PASTE( \
			assertion_failed_##file##_, line)[2 * !!(predicate)-1]

uint32_t log2_u32(uint32_t x);

#endif /* end of include guard SQUASH_UTILS_H */
