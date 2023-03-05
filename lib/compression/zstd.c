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
 * @file         zstd.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_error.h"

#ifdef CONFIG_ZSTD

#	include <zstd.h>

struct SqshZstdContext {
	ZSTD_DCtx *stream;
	ZSTD_outBuffer output;
};

SQSH_STATIC_ASSERT(
		sizeof(sqsh__compression_context_t) >= sizeof(struct SqshZstdContext));

static int
sqsh_zstd_init(void *context, uint8_t *target, size_t target_size) {
	(void)target;
	(void)target_size;
	struct SqshZstdContext *ctx = context;
	ctx->stream = ZSTD_createDCtx();
	if (ctx->stream == NULL) {
		return -SQSH_ERROR_COMPRESSION_INIT;
	}
	ctx->output.dst = target;
	ctx->output.size = target_size;
	ctx->output.pos = 0;

	return 0;
}

static int
sqsh_zstd_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	struct SqshZstdContext *ctx = context;
	ZSTD_inBuffer input = {
			.src = compressed,
			.size = compressed_size,
			.pos = 0,
	};

	while (input.pos < input.size) {
		size_t rv = ZSTD_decompressStream(ctx->stream, &ctx->output, &input);
		if (ZSTD_isError(rv)) {
			return -SQSH_ERROR_COMPRESSION_DECOMPRESS;
		}
	}
	return 0;
}

static int
sqsh_zstd_finish(void *context, uint8_t *target, size_t *target_size) {
	(void)target;
	(void)target_size;
	struct SqshZstdContext *ctx = context;
	ZSTD_freeDCtx(ctx->stream);
	*target_size = ctx->output.pos;
	return 0;
}

static const struct SqshCompressionImpl impl_zstd = {
		.init = sqsh_zstd_init,
		.decompress = sqsh_zstd_decompress,
		.finish = sqsh_zstd_finish,
};

const struct SqshCompressionImpl *const sqsh__impl_zstd = &impl_zstd;
#else
const struct SqshCompressionImpl *const sqsh__impl_zstd = NULL;
#endif
