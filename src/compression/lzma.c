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

#include "../error.h"
#include "../format/compression_options.h"
#include "compression.h"

#define BLOCK_SIZE 8192

static int
lzma_uncompress(uint8_t *dest, size_t *dest_len, const uint8_t *source,
		size_t source_len) {
	lzma_ret rv = LZMA_OK;

	lzma_stream strm = LZMA_STREAM_INIT;

	rv = lzma_alone_decoder(&strm, UINT64_MAX);
	if (rv != LZMA_OK) {
		lzma_end(&strm);
		return rv;
	}

	lzma_action action = LZMA_RUN;

	strm.next_in = source;
	strm.avail_in = source_len;

	strm.next_out = dest;
	strm.avail_out = *dest_len;

	action = LZMA_FINISH;

	*dest_len = strm.avail_out;

	rv = lzma_code(&strm, action);
	lzma_end(&strm);

	return rv;
}

static int
squash_lzma_extract(const union SquashCompressionOptions *options,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	size_t write_chunk_size = BLOCK_SIZE;
	uint8_t *target_buffer = realloc(*target, *target_size + write_chunk_size);
	if (target_buffer == NULL) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}

	rv = lzma_uncompress(&target_buffer[*target_size], &write_chunk_size, compressed,
			compressed_size);

	*target = target_buffer;
	*target_size += write_chunk_size;

	if (rv != LZMA_STREAM_END) {
		return -SQUASH_ERROR_COMPRESSION_DECOMPRESS;
	}
	return rv;
}

const struct SquashExtractorImplementation squash_extractor_lzma = {
		.extract = squash_lzma_extract,
		.default_options = NULL,
};
