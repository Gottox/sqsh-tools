/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : null_handler
 * @created     : Monday May 30, 2021 13:21:55 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"

int
squash_null_stream_init(union SquashDecompressorInfo *info,
		union SquashDecompressorStreamInfo *stream_info, uint8_t *data,
		size_t data_size, off_t read_start) {
	stream_info->null.data = &data[read_start];
	stream_info->null.size = data_size - read_start;
	return 0;
}

int
squash_null_stream_decompress(union SquashDecompressorStreamInfo *stream) {
	return stream->null.size;
}

uint8_t *
squash_null_stream_data(union SquashDecompressorStreamInfo *stream) {
	return stream->null.data;
}

size_t
squash_null_stream_size(union SquashDecompressorStreamInfo *stream) {
	return stream->null.size;
}

int
squash_null_stream_cleanup(union SquashDecompressorStreamInfo *stream) {
	return 0;
}

int
squash_null_init(
		union SquashDecompressorInfo *info, void *options, size_t size) {
	return 0;
}

int
squash_null_cleanup(union SquashDecompressorInfo *info) {
	return 0;
}

struct SquashDecompressorImpl squash_null_deflate = {
		.stream =
				{
						.init = squash_null_stream_init,
						.decompress = squash_null_stream_decompress,
						.data = squash_null_stream_data,
						.size = squash_null_stream_size,
						.cleanup = squash_null_stream_cleanup,
				},

		.init = squash_null_init,
		.cleanup = squash_null_cleanup,
};
