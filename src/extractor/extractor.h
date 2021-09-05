/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : extractor
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include <stddef.h>
#include <stdint.h>

#ifndef EXTRACTOR_H

#define EXTRACTOR_H

struct Squash;

union SquashExtractorOptions {
	const struct SquashExtractorNullOptions *null;
	const struct SquashExtractorGzipOptions *gzip;
};

struct SquashExtractorImplementation {
	int (*init)(union SquashExtractorOptions *options, const void *option_buffer,
			const size_t option_buffer_size);
	int (*extract)(const union SquashExtractorOptions *options,
			uint8_t **target, size_t *target_size, const uint8_t *compressed,
			const size_t compressed_size);
	int (*cleanup)(union SquashExtractorOptions *options);
};

struct SquashExtractor {
	union SquashExtractorOptions options;
	const struct SquashExtractorImplementation *impl;
};

int squash_extractor_init(
		struct Squash *squash, struct SquashExtractor *extractor);

int squash_extractor_extract(struct SquashExtractor *extractor,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);

int squash_extractor_cleanup(struct SquashExtractor *extractor);

#endif /* end of include guard EXTRACTOR_H */
