/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : extractor
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include "../format/compression_options.h"

#include <stddef.h>
#include <stdlib.h>

#ifndef EXTRACTOR_H

#define EXTRACTOR_H

extern const struct SquashExtractorImplementation squash_extractor_null;
extern const struct SquashExtractorImplementation squash_extractor_gzip;
extern const struct SquashExtractorImplementation squash_extractor_lzma;
extern const struct SquashExtractorImplementation squash_extractor_xz;
extern const struct SquashExtractorImplementation squash_extractor_lzo;
extern const struct SquashExtractorImplementation squash_extractor_lz4;
extern const struct SquashExtractorImplementation squash_extractor_zstd;

struct Squash;

struct SquashExtractorImplementation {
	const union SquashCompressionOptions *default_options;
	int (*extract)(const union SquashCompressionOptions *options,
			uint8_t **target, size_t *target_size, const uint8_t *compressed,
			const size_t compressed_size);
};

struct SquashExtractor {
	const union SquashCompressionOptions *options;
	const struct SquashExtractorImplementation *impl;
};

int squash_extractor_init(
		struct Squash *squash, struct SquashExtractor *extractor);

int squash_extractor_extract(struct SquashExtractor *extractor,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);

int squash_extractor_cleanup(struct SquashExtractor *extractor);

#endif /* end of include guard EXTRACTOR_H */
