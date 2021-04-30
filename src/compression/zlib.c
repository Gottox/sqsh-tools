/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : zlib
 * @created     : Friday Apr 30, 2021 20:49:40 CEST
 */

#include <stdlib.h>
#include <zlib.h>

#include "compression.h"

void *
zlib_init() {
	return NULL;
}
int
zlib_fill(void *stream, const uint8_t *data, const int size) {
	return 0;
}
int
zlib_collect(void *stream, uint8_t **data, int *size) {
	return 0;
}
int
zlib_cleanup(void *uncompress_stream) {
	return 0;
}

struct DeflateStream zlib_deflate_stream = {
	.init = zlib_init,
	.fill = zlib_fill,
	.collect = zlib_collect,
	.cleanup = zlib_cleanup,
};
