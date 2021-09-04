/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Friday Apr 30, 2021 20:49:40 CEST
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "../error.h"
#include "../utils.h"
#include "compression.h"
#include "gzip_handler.h"
#define MEM_LEVEL 8
#define METHOD Z_DEFLATED

const static struct SquashGzipOptions default_options = {
		.compression_level = 9,
		.window_size = 15,
		.strategies = 0x01,
};

int
squash_gzip_init(
		union SquashDecompressorInfo *info, void *options, size_t size) {
	int rv = 0;

	if (options == NULL) {
		info->gzip.options = &default_options;
	} else {
		if (size < sizeof(struct SquashGzipOptions)) {
			rv = -SQUASH_ERROR_COMPRESSION_INIT;
			goto err;
		}
		struct SquashGzipOptions *gzip_options = options;
		ENSURE_HOST_ORDER_32(gzip_options->compression_level);
		ENSURE_HOST_ORDER_16(gzip_options->window_size);
		ENSURE_HOST_ORDER_16(gzip_options->strategies);
		info->gzip.options = gzip_options;
	}

	info->gzip.page_size = (size_t)sysconf(_SC_PAGESIZE);

	return 0;
err:
	return rv;
}

int
squash_gzip_stream_init(union SquashDecompressorInfo *info,
		union SquashDecompressorStreamInfo *stream_info, uint8_t *data,
		size_t data_size, off_t read_start) {
	int rv = 0;
	struct SquashGzip *gzip = &info->gzip;
	int window_bits = gzip->options->window_size;

	stream_info->gzip.stream.zalloc = Z_NULL;
	stream_info->gzip.stream.zfree = Z_NULL;
	stream_info->gzip.stream.opaque = Z_NULL;

	stream_info->gzip.gzip = gzip;
	stream_info->gzip.out = NULL;
	stream_info->gzip.out_size = 0;

	if (read_start > data_size) {
		return -SQUASH_ERROR_GZIP_READ_AFTER_END;
	}

	if (read_start != 0) {
		window_bits = -window_bits;
	}

	rv = inflateInit2(&stream_info->gzip.stream, window_bits);
	if (rv != Z_OK) {
		return -SQUASH_ERROR_COMPRESSION_STREAM_INIT;
	}

	// rv = gzip_read_header(&stream_info->gzip, data, data_size);
	stream_info->gzip.stream.next_in = &data[read_start];
	stream_info->gzip.stream.avail_in = data_size - read_start;
	return 0;
}

int
squash_gzip_stream_decompress(union SquashDecompressorStreamInfo *info) {
	struct SquashGzipStream *stream = &info->gzip;
	int block_size = stream->gzip->page_size;

	int rv = 0;
	uint8_t *tmp = realloc(stream->out, stream->out_size + block_size);
	if (tmp == NULL) {
		return -1;
	}
	stream->out = tmp;

	stream->stream.avail_out = block_size;
	stream->stream.next_out = &stream->out[stream->out_size];

	rv = inflate(&stream->stream, Z_SYNC_FLUSH);
	switch (rv) {
	case Z_OK:
		stream->out_size += block_size;
		break;
	case Z_STREAM_END:
		stream->out_size = stream->stream.total_out;
		break;
	default:
		return -1;
	}

	return stream->stream.total_out;
}

uint8_t *
squash_gzip_stream_data(union SquashDecompressorStreamInfo *info) {
	return info->gzip.out;
}

size_t
squash_gzip_stream_size(union SquashDecompressorStreamInfo *info) {
	return info->gzip.out_size;
}

int
squash_gzip_stream_cleanup(union SquashDecompressorStreamInfo *info) {
	struct SquashGzipStream *stream = &info->gzip;
	int rv = 0;

	rv = inflateEnd(&stream->stream);
	if (rv != Z_OK) {
		return -SQUASH_ERROR_COMPRESSION_STREAM_CLEANUP;
	}

	free(stream->out);

	return 0;
}

int
squash_gzip_cleanup(union SquashDecompressorInfo *de) {
	return 0;
}

struct SquashDecompressorImpl squash_gzip_deflate = {
		.stream =
				{
						.init = squash_gzip_stream_init,
						.decompress = squash_gzip_stream_decompress,
						.data = squash_gzip_stream_data,
						.size = squash_gzip_stream_size,
						.cleanup = squash_gzip_stream_cleanup,
				},

		.init = squash_gzip_init,
		.cleanup = squash_gzip_cleanup,
};
