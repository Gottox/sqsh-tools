/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include "../format/compression_options.h"

#include <stddef.h>
#include <stdlib.h>

#ifndef EXTRACTOR_H

#define EXTRACTOR_H

extern const struct SquashCompressionImplementation squash_compression_null;
extern const struct SquashCompressionImplementation squash_compression_gzip;
extern const struct SquashCompressionImplementation squash_compression_lzma;
extern const struct SquashCompressionImplementation squash_compression_xz;
extern const struct SquashCompressionImplementation squash_compression_lzo;
extern const struct SquashCompressionImplementation squash_compression_lz4;
extern const struct SquashCompressionImplementation squash_compression_zstd;

struct Squash;

struct SquashCompressionImplementation {
	const union SquashCompressionOptions *default_options;
	int (*extract)(const union SquashCompressionOptions *options,
			uint8_t **target, size_t *target_size, const uint8_t *compressed,
			const size_t compressed_size);
};

struct SquashCompression {
	const union SquashCompressionOptions *options;
	const struct SquashCompressionImplementation *impl;
};

int squash_compression_init(
		struct Squash *squash, struct SquashCompression *compression);

int squash_compression_extract(struct SquashCompression *extractor,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);

int squash_compression_cleanup(struct SquashCompression *extractor);

#endif /* end of include guard EXTRACTOR_H */
