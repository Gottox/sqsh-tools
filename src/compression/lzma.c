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
 * @file         lzma.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_error.h"

#ifdef CONFIG_LZMA

#	include <lzma.h>
#	include <string.h>

SQSH_STATIC_ASSERT(sizeof(sqsh__compression_context_t) >= sizeof(lzma_stream));

static lzma_stream proto_stream = LZMA_STREAM_INIT;

enum SqshLzmaType {
	LZMA_TYPE_ALONE,
	LZMA_TYPE_XZ,
};

static int
sqsh_lzma_init(
		void *context, uint8_t *target, size_t target_size,
		enum SqshLzmaType type) {
	lzma_stream *stream = context;
	memcpy(stream, &proto_stream, sizeof(lzma_stream));

	lzma_ret ret;
	if (type == LZMA_TYPE_ALONE) {
		ret = lzma_alone_decoder(stream, UINT64_MAX);
	} else {
		ret = lzma_stream_decoder(stream, UINT64_MAX, 0);
	}

	if (ret != LZMA_OK) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	stream->next_out = target;
	stream->avail_out = target_size;

	return 0;
}

static int
sqsh_lzma_init_xz(void *context, uint8_t *target, size_t target_size) {
	return sqsh_lzma_init(context, target, target_size, LZMA_TYPE_XZ);
}

static int
sqsh_lzma_init_alone(void *context, uint8_t *target, size_t target_size) {
	return sqsh_lzma_init(context, target, target_size, LZMA_TYPE_ALONE);
}

static int
sqsh_lzma_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	lzma_stream *stream = context;

	stream->next_in = compressed;
	stream->avail_in = compressed_size;

	lzma_ret ret = lzma_code(stream, LZMA_RUN);
	if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	return 0;
}

static int
sqsh_lzma_finish(void *context, uint8_t *target, size_t *target_size) {
	(void)target;
	lzma_stream *stream = context;
	stream->next_in = NULL;
	stream->avail_in = 0;

	lzma_ret ret = lzma_code(stream, LZMA_FINISH);

	if (ret != LZMA_STREAM_END) {
		return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
	}

	*target_size = stream->total_out;
	return 0;
}

static const struct SqshCompressionImpl impl_xz = {
		.init = sqsh_lzma_init_xz,
		.decompress = sqsh_lzma_decompress,
		.finish = sqsh_lzma_finish,
};

const struct SqshCompressionImpl *sqsh__xz_impl = &impl_xz;

static const struct SqshCompressionImpl impl_lzma = {
		.init = sqsh_lzma_init_alone,
		.decompress = sqsh_lzma_decompress,
		.finish = sqsh_lzma_finish,
};

const struct SqshCompressionImpl *sqsh__lzma_impl = &impl_lzma;
#else
const struct SqshCompressionImpl *sqsh__lzma_impl = NULL;
const struct SqshCompressionImpl *sqsh__xz_impl = NULL;
#endif
