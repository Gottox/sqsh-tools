/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression_options_internal
 * @created     : Friday Sep 17, 2021 11:02:51 CEST
 */

#include "compression_options.h"

#ifndef COMPRESSION_OPTIONS_INTERNAL_H

#define COMPRESSION_OPTIONS_INTERNAL_H

struct SquashCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};

struct SquashCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};

struct SquashCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};

struct SquashCompressionOptionsZstd {
	uint32_t compression_level;
};

struct SquashCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};

union SquashCompressionOptions {
	struct SquashCompressionOptionsGzip gzip;
	struct SquashCompressionOptionsXz xz;
	struct SquashCompressionOptionsLz4 lz4;
	struct SquashCompressionOptionsZstd zstd;
	struct SquashCompressionOptionsLzo lzo;
};

#endif /* end of include guard COMPRESSION_OPTIONS_INTERNAL_H */
