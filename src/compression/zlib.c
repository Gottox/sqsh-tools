/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         gzip.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_error.h"

#include <zlib.h>

static int
sqsh_zlib_init(void *context, uint8_t *target, size_t target_size) {
	z_stream *stream = context;
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = Z_NULL;
	stream->next_out = target;
	stream->avail_out = target_size;

	if (inflateInit(stream) != Z_OK) {
		return -SQSH_ERROR_COMPRESSION_INIT;
	} else {
		return 0;
	}
}

static int
sqsh_zlib_decompress(
		void *context, const uint8_t *compressed, const size_t compressed_size,
		bool end) {
	int rv = 0;
	z_stream *stream = context;
	stream->next_in = (Bytef *)compressed;
	stream->avail_in = compressed_size;
	rv = inflate(stream, end ? Z_FINISH : Z_NO_FLUSH);

	if (end && rv != Z_STREAM_END) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	} else if (!end && rv != Z_OK) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	} else {
		return 0;
	}
}

static int
sqsh_zlib_finish(void *context, size_t *written_size) {
	z_stream *stream = context;

	*written_size = stream->total_out;
	return 0;
}

int
sqsh_extract_zlib(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	z_stream stream;
	int rv = 0;

	if ((rv = sqsh_zlib_init(&stream, target, *target_size)) != Z_OK) {
		return rv;
	}

	if ((rv = sqsh_zlib_decompress(
				 &stream, compressed, compressed_size, true)) != 0) {
		return rv;
	}

	if ((rv = sqsh_zlib_finish(&stream, target_size)) != 0) {
		return rv;
	}

	return rv;
}
