/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : null
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include "../error.h"
#include "../format/compression_options.h"
#include "compression.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static int
squash_null_extract(const union SquashCompressionOptions *options,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	size_t new_target_size = *target_size + compressed_size;
	uint8_t *new_target = realloc(*target, new_target_size);
	if (new_target == NULL) {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}

	memcpy(&new_target[*target_size], compressed, compressed_size);

	*target = new_target;
	*target_size = new_target_size;

	return 0;
}

const struct SquashExtractorImplementation squash_extractor_null = {
		.extract = squash_null_extract,
		.default_options = NULL,
};
