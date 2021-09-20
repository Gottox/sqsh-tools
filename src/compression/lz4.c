/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lz4
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include <lz4.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../data/compression_options.h"
#include "../error.h"
#include "compression.h"

static int
squash_lz4_extract(const union SquashCompressionOptions *options,
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	return LZ4_decompress_safe(
			(char *)compressed, (char *)target, compressed_size, *target_size);
}

const struct SquashCompressionImplementation squash_compression_lz4 = {
		.extract = squash_lz4_extract,
};
