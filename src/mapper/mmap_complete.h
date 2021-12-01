/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mmap_complete
 * @created     : Sunday Nov 21, 2021 15:58:40 CET
 */

#include <stddef.h>
#include <stdint.h>

#ifndef MMAP_COMPLETE_H

#define MMAP_COMPLETE_H

struct HsqsMapperMmapComplete {
	uint8_t *data;
	size_t size;
};

struct HsqsMapMmapComplete {
	uint8_t *data;
	size_t size;
};

#endif /* end of include guard MMAP_COMPLETE_H */
