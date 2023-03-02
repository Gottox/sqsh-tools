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
 * @file         comression.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_primitive_private.h"

const struct SqshCompressionImpl *
compression_by_id(int id) {
	switch ((enum SqshSuperblockCompressionId)id) {
	case SQSH_COMPRESSION_GZIP:
		return sqsh__zlib_impl;
	case SQSH_COMPRESSION_LZMA:
		return sqsh__lzma_impl;
	case SQSH_COMPRESSION_XZ:
		return sqsh__xz_impl;
	case SQSH_COMPRESSION_LZO:
		return sqsh__lzo_impl;
	case SQSH_COMPRESSION_LZ4:
		return sqsh__lz4_impl;
	case SQSH_COMPRESSION_ZSTD:
		return sqsh__zstd_impl;
	default:
		return NULL;
	}
}

int
sqsh__compression_init(
		struct SqshCompression *compression, int compression_id,
		size_t block_size) {
	const struct SqshCompressionImpl *impl = compression_by_id(compression_id);
	if (impl == NULL) {
		return -SQSH_ERROR_COMPRESSION_UNSUPPORTED;
	}
	compression->impl = impl;
	compression->block_size = block_size;
	return 0;
}

int
sqsh__compression_decompress_to_buffer(
		const struct SqshCompression *compression, struct SqshBuffer *buffer,
		const uint8_t *compressed, const size_t compressed_size) {
	int rv = 0;
	size_t max_size = compression->block_size;
	size_t size = 0;
	uint8_t *decompressed = NULL;
	const struct SqshCompressionImpl *impl = compression->impl;
	sqsh__compression_context_t compression_context = {0};

	rv = sqsh__buffer_add_capacity(buffer, &decompressed, max_size);
	if (rv < 0) {
		return rv;
	}

	rv = impl->init(&compression_context, decompressed, max_size);
	if (rv < 0) {
		return rv;
	}
	size = max_size;
	// TODO: check return value of decompress
	impl->decompress(&compression_context, compressed, compressed_size);
	rv = impl->finish(&compression_context, decompressed, &size);
	if (rv < 0) {
		return rv;
	}

	rv = sqsh__buffer_add_size(buffer, size);
	return rv;
}

int
sqsh__compression_cleanup(struct SqshCompression *compression) {
	compression->impl = NULL;
	compression->block_size = 0;
	return 0;
}
