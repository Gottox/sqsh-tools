/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : datablock
 * @created     : Friday Sep 24, 2021 19:03:20 CEST
 */

#include "datablock_internal.h"
#include <endian.h>

uint32_t
squash_data_datablock_size(const struct SquashDatablockSize *datablock_size) {
	return le32toh(datablock_size->size) & ~(1 << 24);
}

bool
squash_data_datablock_is_compressed(
		const struct SquashDatablockSize *datablock_size) {
	return !(le32toh(datablock_size->size) & (1 << 24));
}
