/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Friday Apr 30, 2021 20:49:40 CEST
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "../utils.h"
#include "../error.h"
#include "compression.h"
#include "gzip_handler.h"
#define MEM_LEVEL 8
#define METHOD Z_DEFLATED

const static struct SquashGzipOptions default_options = {
		.compression_level = 9,
		.window_size = 15,
		.strategies = 0x01,
};

static int
map_strategies(uint8_t squash_id) {
	switch (squash_id) {
	case 0x00:
	case 0x01:
		return Z_DEFAULT_STRATEGY;
	case 0x02:
		return Z_FILTERED;
	case 0x04:
		return Z_HUFFMAN_ONLY;
	case 0x08:
		return Z_RLE;
	case 0x10:
		return Z_FIXED;
	default:
		return -1;
	}
}

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
squash_gzip_decompress(union SquashDecompressorInfo *de, uint8_t **out,
		size_t *out_size, uint8_t *in, const off_t in_offset,
		const size_t in_size) {
	int rv = 0;
	struct SquashGzip *gzip = &de->gzip;
	z_stream stream = {0};
	int strategy = map_strategies(gzip->options->strategies);
	uint8_t *tmp = NULL;

	*out = NULL;
	*out_size = 0;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	rv = deflateInit2(&stream, gzip->options->compression_level, METHOD,
			gzip->options->window_size, MEM_LEVEL, strategy);
	if (rv != Z_OK) {
		return -1;
	}

	stream.avail_in = in_size - in_offset;
	stream.next_in = &in[in_offset];

	do {
		stream.avail_out = de->gzip.page_size;
		stream.next_out = &(*out)[*out_size];
		*out_size += de->gzip.page_size;

		tmp = realloc(*out, *out_size);
		if (tmp == NULL) {
			free(*out);
			*out = NULL;
			return -errno;
		}
		*out = tmp;

		rv = deflate(&stream, Z_FINISH);
	} while (stream.avail_out == 0);

	*out_size = stream.avail_out;

	return 0;
}

int
squash_gzip_cleanup(union SquashDecompressorInfo *de) {
	return 0;
}

struct SquashDecompressorImpl squash_gzip_deflate = {
		.init = squash_gzip_init,
		.decompress = squash_gzip_decompress,
		.cleanup = squash_gzip_cleanup,
};
