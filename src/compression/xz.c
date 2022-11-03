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
 * @file         xz.c
 */

#include <lzma.h>
#include <sqsh_compression.h>
#include <sqsh_data.h>
#include <sqsh_error.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zconf.h>

static int
sqsh_xz_extract(
		const union SqshCompressionOptions *options, size_t options_size,
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	if (options != NULL && options_size != SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	int rv = 0;
	size_t compressed_pos = 0;
	size_t target_pos = 0;
	uint64_t memlimit = UINT64_MAX;

	rv = lzma_stream_buffer_decode(
			&memlimit, 0, NULL, compressed, &compressed_pos, compressed_size,
			target, &target_pos, *target_size);

	*target_size = target_pos;

	if (rv != LZMA_OK) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	if (compressed_pos != compressed_size) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}
	return rv;
}

const struct SqshCompressionImplementation sqsh_compression_xz = {
		.extract = sqsh_xz_extract,
};
