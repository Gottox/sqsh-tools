/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compressor
 * @created     : Wednesday Dec 22, 2021 18:28:21 CET
 */

#ifndef COMPRESSOR_H

#define COMPRESSOR_H

#include "../utils.h"
#include <stddef.h>
#include <stdint.h>

struct HsqsCompressionImplementation;
struct HsqsBuffer;

struct HsqsCompressor {
	const struct HsqsCompressionImplementation *impl;
};

HSQS_NO_UNUSED int
hsqs_compressor_init(struct HsqsCompressor *compressor, int compressor_id);
HSQS_NO_UNUSED int hsqs_compressor_write_to_buffer(
		struct HsqsCompressor *compressor, const uint8_t *source,
		const size_t source_size, struct HsqsBuffer *buffer);
int hsqs_compressor_cleanup(struct HsqsCompressor *compressor);

#endif /* end of include guard COMPRESSOR_H */
