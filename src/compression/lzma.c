/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lzma
 * @created     : Sunday Sep 05, 2021 11:09:51 CEST
 */

#include <lzma.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zconf.h>

#include "../data/compression_options.h"
#include "../error.h"
#include "compression.h"

static int
squash_lzma_extract(const union SquashCompressionOptions *options,
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	lzma_ret rv = LZMA_OK;

	lzma_stream strm = LZMA_STREAM_INIT;

	rv = lzma_alone_decoder(&strm, UINT64_MAX);
	if (rv != LZMA_OK) {
		lzma_end(&strm);
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}

	lzma_action action = LZMA_RUN;

	strm.next_in = compressed;
	strm.avail_in = compressed_size;

	strm.next_out = target;
	strm.avail_out = *target_size;

	action = LZMA_FINISH;

	rv = lzma_code(&strm, action);

	*target_size = strm.avail_out;
	lzma_end(&strm);

	return 0;
}

const struct SquashCompressionImplementation squash_compression_lzma = {
		.extract = squash_lzma_extract,
};
