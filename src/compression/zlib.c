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

#ifdef CONFIG_ZLIB

#	include <zlib.h>

SQSH_STATIC_ASSERT(sizeof(sqsh__compression_context_t) >= sizeof(z_stream));

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
	}

	return 0;
}

static int
sqsh_zlib_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv;
	z_stream *stream = context;
	stream->next_in = (Bytef *)compressed;
	stream->avail_in = compressed_size;
	rv = inflate(stream, Z_NO_FLUSH);

	if (rv != Z_STREAM_END && rv != Z_OK) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}
	return 0;
}

static int
sqsh_zlib_finish(void *context, uint8_t *target, size_t *target_size) {
	(void)target;

	int rv;
	z_stream *stream = context;
	stream->next_in = Z_NULL;
	stream->avail_in = 0;

	rv = inflate(stream, Z_FINISH);

	if (rv != Z_STREAM_END) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	*target_size = stream->total_out;
	return 0;
}

static const struct SqshCompressionImpl impl = {
		.init = sqsh_zlib_init,
		.decompress = sqsh_zlib_decompress,
		.finish = sqsh_zlib_finish,
};

const struct SqshCompressionImpl *sqsh__zlib_impl = &impl;
#else
const struct SqshCompressionImpl *sqsh__zlib_impl = NULL;
#endif
