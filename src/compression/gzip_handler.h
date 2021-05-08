/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Monday May 03, 2021 10:43:15 CEST
 */

#include <stdint.h>
#include <stdlib.h>

#ifndef GZIP_HANDLER_H

#define GZIP_HANDLER_H

struct SquashGzipOptions {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};

struct SquashGzip {
	const struct SquashGzipOptions *options;
	size_t page_size;
};

#endif /* end of include guard GZIP_HANDLER_H */
