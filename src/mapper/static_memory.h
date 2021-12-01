/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mmap_complete
 * @created     : Sunday Nov 21, 2021 15:58:40 CET
 */

#include <stddef.h>
#include <stdint.h>

#ifndef STATIC_H

#define STATIC_H

struct HsqsMapperStaticMemory {
	const uint8_t *data;
	size_t size;
};

struct HsqsMapStaticMemory {
	const uint8_t *data;
	size_t size;
};
#endif /* end of include guard STATIC_H */
