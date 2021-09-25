/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : datablock
 * @created     : Friday Sep 24, 2021 19:01:07 CEST
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef DATABLOCK_H

#define DATABLOCK_H

struct SquashDatablockSize;

uint32_t squash_data_datablock_size(
		const struct SquashDatablockSize *datablock_size);
bool squash_data_datablock_is_compressed(
		const struct SquashDatablockSize *datablock_size);

#endif /* end of include guard DATABLOCK_H */
