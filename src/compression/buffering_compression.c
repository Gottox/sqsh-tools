/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : buffering_compression
 * @created     : Wednesday Mar 01, 2023 09:32:00 CET
 */

#include "../../include/sqsh_compression_private.h"

SQSH_STATIC_ASSERT(
		sizeof(sqsh__compression_context_t) >=
		sizeof(struct SqshBufferingCompression));

int
sqsh__buffering_compression_init(
		void *context, uint8_t *target, size_t target_size) {
	(void)target;
	(void)target_size;
	int rv = 0;
	struct SqshBufferingCompression *buffering = context;

	rv = sqsh__buffer_init(&buffering->buffer);
	if (rv < 0) {
		goto out;
	}
	buffering->compressed = NULL;
	buffering->compressed_size = 0;

out:
	return rv;
}
int
sqsh__buffering_compression_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	struct SqshBufferingCompression *buffering = context;
	if (buffering->compressed == NULL &&
		sqsh__buffer_size(&buffering->buffer) == 0) {
		buffering->compressed = compressed;
		buffering->compressed_size = compressed_size;
		return 0;
	} else if (sqsh__buffer_size(&buffering->buffer) == 0) {
		rv = sqsh__buffer_append(
				&buffering->buffer, buffering->compressed,
				buffering->compressed_size);
		if (rv < 0) {
			return rv;
		}
	}

	rv = sqsh__buffer_append(&buffering->buffer, compressed, compressed_size);
	if (rv < 0) {
		return rv;
	}
	buffering->compressed = sqsh__buffer_data(&buffering->buffer);
	buffering->compressed_size = sqsh__buffer_size(&buffering->buffer);

	return rv;
}

const uint8_t *
sqsh__buffering_compression_data(void *context) {
	return ((struct SqshBufferingCompression *)context)->compressed;
}

size_t
sqsh__buffering_compression_size(void *context) {
	return ((struct SqshBufferingCompression *)context)->compressed_size;
}

int
sqsh__buffering_compression_cleanup(void *context) {
	struct SqshBufferingCompression *buffering = context;
	return sqsh__buffer_cleanup(&buffering->buffer);
}
