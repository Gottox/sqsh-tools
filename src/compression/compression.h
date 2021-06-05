/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compressor
 * @created     : Friday Apr 30, 2021 19:33:51 CEST
 */

#include "../metablock.h"
#include "gzip_handler.h"
#include "null_handler.h"

#include <stdint.h>
#include <stdlib.h>
#include <zlib.h>

#ifndef COMPRESSION_H

#define COMPRESSION_H

struct Squash;

union SquashDecompressorInfo {
	void *null;
	struct SquashGzip gzip;
};

union SquashDecompressorStreamInfo {
	struct SquashGzipStream gzip;
	struct SquashNullStream null;
};

struct SquashDecompressorStreamImpl {
	int (*init)(union SquashDecompressorInfo *info,
			union SquashDecompressorStreamInfo *stream_info,
			uint8_t *data, size_t data_size, off_t read_start);
	int (*decompress)(union SquashDecompressorStreamInfo *stream);
	uint8_t *(*data)(union SquashDecompressorStreamInfo *stream);
	size_t (*size)(union SquashDecompressorStreamInfo *stream);
	int (*cleanup)(union SquashDecompressorStreamInfo *stream);
};

struct SquashDecompressorImpl {
	struct SquashDecompressorStreamImpl stream;

	int (*init)(union SquashDecompressorInfo *info, void *options, size_t size);
	int (*cleanup)(union SquashDecompressorInfo *info);
};

struct SquashDecompressor {
	union SquashDecompressorInfo info;
	struct SquashMetablock compression_info_block;
	struct SquashDecompressorImpl *impl;
};

int squash_decompressor_init(
		struct SquashDecompressor *de, struct Squash *squash);

int squash_decompressor_cleanup(struct SquashDecompressor *de);
#endif /* end of include guard COMPRESSION_H */
