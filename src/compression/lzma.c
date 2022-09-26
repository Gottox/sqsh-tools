/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         lzma.c
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
sqsh_lzma_extract(
		const union SqshCompressionOptions *options, size_t options_size,
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	// LZMA has no compression options
	if (options != NULL || options_size != 0) {
		// TODO: More specific error code
		return -HSQS_ERROR_COMPRESSION_DECOMPRESS;
	}
	lzma_ret rv = LZMA_OK;

	lzma_stream strm = LZMA_STREAM_INIT;

	rv = lzma_alone_decoder(&strm, UINT64_MAX);
	if (rv != LZMA_OK) {
		lzma_end(&strm);
		return -HSQS_ERROR_COMPRESSION_DECOMPRESS;
	}

	lzma_action action = LZMA_RUN;

	strm.next_in = compressed;
	strm.avail_in = compressed_size;

	strm.next_out = target;
	strm.avail_out = *target_size;

	action = LZMA_FINISH;

	if (lzma_code(&strm, action) != LZMA_OK) {
		rv = -HSQS_ERROR_COMPRESSION_DECOMPRESS;
	}

	*target_size = strm.avail_out;
	lzma_end(&strm);

	return rv;
}

const struct SqshCompressionImplementation sqsh_compression_lzma = {
		.extract = sqsh_lzma_extract,
};
