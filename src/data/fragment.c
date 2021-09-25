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

const struct SquashDatablockSize *
squash_data_fragment_size_info(const struct SquashFragment *fragment) {
	return &fragment->size;
}
