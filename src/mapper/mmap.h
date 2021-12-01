/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mmap_complete
 * @created     : Sunday Nov 21, 2021 15:58:40 CET
 */

#include <stddef.h>
#include <stdint.h>

#ifndef MMAP_H

#define MMAP_H

struct HsqsMapperMmap {
	int fd;
	size_t size;
};

struct HsqsMapMmap {
	uint8_t *data;
	size_t size;
};

#endif /* end of include guard MMAP_H */
