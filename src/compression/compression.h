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

struct SquashSuperblock;

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

int squash_compression_init(struct SquashCompression *compression,
		const struct SquashSuperblock *superblock);

int squash_compression_extract(const struct SquashCompression *compression,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);

int squash_compression_cleanup(struct SquashCompression *compression);

#endif /* end of include guard EXTRACTOR_H */
