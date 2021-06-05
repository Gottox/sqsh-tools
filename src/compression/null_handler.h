/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : null_handler
 * @created     : Monday May 30, 2021 13:21:55 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <zlib.h>

#ifndef NULL_HANDLER_H

#define NULL_HANDLER_H

struct SquashNullStream {
	uint8_t *data;
	size_t size;
};

#endif /* end of include guard NULL_HANDLER_H */
