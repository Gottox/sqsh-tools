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

static int
sqsh_zstd_finish(void *context, uint8_t *target, size_t *target_size) {
	const uint8_t *compressed = sqsh__buffering_compression_data(context);
	const size_t compressed_size = sqsh__buffering_compression_size(context);

	int rv = ZSTD_decompress(target, *target_size, compressed, compressed_size);
	*target_size = rv;

	if (ZSTD_isError(rv)) {
		rv = -SQSH_ERROR_COMPRESSION_DECOMPRESS;
		goto out;
	}

out:
	sqsh__buffering_compression_cleanup(context);
	return rv;
}

static const struct SqshCompressionImpl impl = {
		.init = sqsh__buffering_compression_init,
		.decompress = sqsh__buffering_compression_decompress,
		.finish = sqsh_zstd_finish,
};

const struct SqshCompressionImpl *sqsh__zstd_impl = &impl;
#else
const struct SqshCompressionImpl *sqsh__zstd_impl = NULL;
#endif
