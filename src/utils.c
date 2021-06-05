/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : utils
 * @created     : Saturday Jun 05, 2021 20:20:07 CEST
 */

#include "utils.h"

uint32_t
log2_u32(uint32_t x) {
	return sizeof(uint32_t) * 8 - 1 - __builtin_clz(x);
}
