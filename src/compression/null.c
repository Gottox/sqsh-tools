/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : null
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include "../data/compression_options.h"
#include "../error.h"
#include "compression.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static int
squash_null_extract(const union SquashCompressionOptions *options,
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	memcpy(target, compressed, compressed_size);

	return 0;
}

const struct SquashCompressionImplementation squash_compression_null = {
		.extract = squash_null_extract,
};
