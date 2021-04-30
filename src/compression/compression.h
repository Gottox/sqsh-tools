/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compressor
 * @created     : Friday Apr 30, 2021 19:33:51 CEST
 */

#ifndef COMPRESSION_H

#define COMPRESSION_H

#include <stdint.h>

enum CompressionId {
	SQUASH_COMPRESSION_NONE = 0,
	SQUASH_COMPRESSION_GZIP = 1,
	SQUASH_COMPRESSION_LZMA = 2,
	SQUASH_COMPRESSION_LZO = 3,
	SQUASH_COMPRESSION_XZ = 4,
	SQUASH_COMPRESSION_LZ4 = 5,
	SQUASH_COMPRESSION_ZSTD = 6,
};

struct DeflateStream {
	void *(*init)();
	int (*fill)(void *stream, const uint8_t *data, const int size);
	int (*collect)(void *stream, uint8_t **data, int *size);
	int (*cleanup)(void *uncompress_stream);
};

extern struct DeflateStream null_deflate_stream;
extern struct DeflateStream zlib_deflate_stream;

#endif /* end of include guard COMPRESSION_H */
