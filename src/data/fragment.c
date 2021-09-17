/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment
 * @created     : Friday Sep 17, 2021 09:23:55 CEST
 */

#include "fragment_internal.h"

#include <endian.h>

uint64_t
squash_data_fragment_start(const struct SquashFragment *fragment) {
	return le64toh(fragment->start);
}
uint32_t
squash_data_fragment_size(const struct SquashFragment *fragment) {
	return le32toh(fragment->size);
}
