/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include "gzip.h"
#include "../error.h"
#include "extractor.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

const static struct SquashExtractorGzipOptions default_options = {
	.compression_level = 9,
	.window_size = 15,
	.strategies = 0x01,
};

int
squash_gzip_init(union SquashExtractorOptions *options, const void *option_buffer,
		const size_t option_buffer_size) {
	int rv = 0;

	if (option_buffer == NULL) {
		options->gzip = &default_options;
	} else {
		if (option_buffer_size < sizeof(struct SquashExtractorGzipOptions)) {
			rv = -SQUASH_ERROR_COMPRESSION_INIT;
			goto err;
		}
		const struct SquashExtractorGzipOptions *gzip_options = option_buffer;
		options->gzip = gzip_options;
	}

	return 0;
err:
	return rv;

}

static int
squash_gzip_extract(const union SquashExtractorOptions *options,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	return -SQUASH_ERROR_TODO;
}

int
squash_gzip_cleanup(union SquashExtractorOptions *options) {
	return 0;
}

const struct SquashExtractorImplementation squash_extractor_gzip = {
		.init = squash_gzip_init,
		.extract = squash_gzip_extract,
		.cleanup = squash_gzip_cleanup,
};
