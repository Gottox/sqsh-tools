/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zconf.h>
#include <zlib.h>

#include "../error.h"
#include "../format/compression_options.h"
#include "compression.h"

#define BLOCK_SIZE 8192

static int
squash_gzip_extract(const union SquashCompressionOptions *options,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	size_t write_chunk_size = BLOCK_SIZE;
	*target = realloc(*target, *target_size + write_chunk_size);
	if (*target == NULL) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}

	rv = uncompress(&(*target)[*target_size], &write_chunk_size, compressed,
			compressed_size);

	*target_size += write_chunk_size;

	if (rv != Z_OK) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}
	return rv;
}

int
squash_gzip_cleanup(union SquashCompressionOptions *options) {
	return 0;
}

const struct SquashCompressionImplementation squash_compression_gzip = {
		.extract = squash_gzip_extract,
		.default_options = NULL,
};
