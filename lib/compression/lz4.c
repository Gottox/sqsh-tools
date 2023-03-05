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
 * @file         lz4.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_error.h"

#ifdef CONFIG_LZ4

#	include <lz4.h>

struct SqshLz4Context {
	LZ4_streamDecode_t *stream;
	uint8_t *target;
	size_t target_size;
	sqsh_index_t offset;
};

SQSH_STATIC_ASSERT(
		sizeof(sqsh__compression_context_t) >= sizeof(LZ4_streamDecode_t));

static int
sqsh_lz4_init(void *context, uint8_t *target, size_t target_size) {
	struct SqshLz4Context *ctx = context;
	ctx->stream = LZ4_createStreamDecode();
	ctx->target = target;
	ctx->target_size = target_size;
	ctx->offset = 0;

	return 0;
}

static int
sqsh_lz4_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	struct SqshLz4Context *ctx = context;

	int size = LZ4_decompress_safe_continue(
			ctx->stream, (const char *)compressed, (char *)ctx->target,
			compressed_size, ctx->target_size - ctx->offset);
	ctx->offset += size;
	return 0;
}

static int
sqsh_lz4_finish(void *context, uint8_t *target, size_t *target_size) {
	(void)target;
	(void)target_size;
	struct SqshLz4Context *ctx = context;
	LZ4_freeStreamDecode(ctx->stream);
	return 0;
}

static const struct SqshCompressionImpl impl_lz4 = {
		.init = sqsh_lz4_init,
		.decompress = sqsh_lz4_decompress,
		.finish = sqsh_lz4_finish,
};

const struct SqshCompressionImpl *const sqsh__impl_lz4 = &impl_lz4;
#else
const struct SqshCompressionImpl *const sqsh__impl_lz4 = NULL;
#endif
