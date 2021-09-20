/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Monday Sep 20, 2021 13:41:58 CEST
 */

#include "../data/compression_options.h"

#include <stddef.h>
#include <stdint.h>

#ifndef COMPRESSION_H

#define COMPRESSION_H

struct SquashCompressionImplementation {
	int (*extract)(const union SquashCompressionOptions *options,
			uint8_t *target, size_t *target_size, const uint8_t *compressed,
			const size_t compressed_size);
};

#endif /* end of include guard COMPRESSION_H */
