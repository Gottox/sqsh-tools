/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lzo
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include <lzo/lzo1x.h>
#include <lzo/lzoconf.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zconf.h>

#include "../error.h"
#include "../format/compression_options.h"
#include "compression.h"

#define BLOCK_SIZE 8192

static int
squash_lzo_extract(const union SquashCompressionOptions *options,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	size_t write_chunk_size = BLOCK_SIZE;
	uint8_t *target_buffer = realloc(*target, *target_size + write_chunk_size);
	if (target_buffer == NULL) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}

	rv = lzo1x_decompress_safe(compressed, compressed_size,
			&target_buffer[*target_size], &write_chunk_size, NULL);

	*target = target_buffer;
	*target_size += write_chunk_size;

	if (rv != LZO_E_OK) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}
	return rv;
}

int
squash_lzo_cleanup(union SquashCompressionOptions *options) {
	return 0;
}

const struct SquashExtractorImplementation squash_extractor_lzo = {
		.extract = squash_lzo_extract,
		.default_options = NULL,
};
