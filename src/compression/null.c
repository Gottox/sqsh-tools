/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : zlib
 * @created     : Friday Apr 30, 2021 20:49:40 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"

struct NullDeflateStream {
	uint8_t *data;
	size_t size;
};

void *
null_init() {
	return calloc(1, sizeof(struct NullDeflateStream));
}

int
null_fill(void *stream, const uint8_t *data, const int size) {
	uint8_t *realloc_data;
	struct NullDeflateStream *null_stream = (struct NullDeflateStream *)stream;
	realloc_data = realloc(null_stream->data, null_stream->size + size);
	if (realloc_data == NULL) {
		return -1;
	}
	memcpy(&realloc_data[null_stream->size], data, size);
	null_stream->data = realloc_data;

	return size;
}

int
null_collect(void *stream, uint8_t **data, int *size) {
	struct NullDeflateStream *null_stream = (struct NullDeflateStream *)stream;

	*data = null_stream->data;
	null_stream->data = NULL;
	*size = null_stream->size;
	null_stream->size = 0;

	return 0;
}

int
null_cleanup(void *stream) {
	struct NullDeflateStream *null_stream = (struct NullDeflateStream *)stream;

	free(null_stream->data);
	free(null_stream);

	return 0;
}

struct DeflateStream null_deflate_stream = {
	.init = null_init,
	.fill = null_fill,
	.collect = null_collect,
	.cleanup = null_cleanup,
};
