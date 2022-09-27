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
 * @file         null.c
 */

#include "compression.h"
#include "../context/superblock_context.h"
#include "../error.h"
#include "../sqsh.h"
#include "data/superblock.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef CONFIG_ZLIB
extern const struct SqshCompressionImplementation sqsh_compression_zlib;
#endif
#ifdef CONFIG_LZMA
extern const struct SqshCompressionImplementation sqsh_compression_lzma;
extern const struct SqshCompressionImplementation sqsh_compression_xz;
#endif
#ifdef CONFIG_LZO
extern const struct SqshCompressionImplementation sqsh_compression_lzo;
#endif
#ifdef CONFIG_LZ4
extern const struct SqshCompressionImplementation sqsh_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
extern const struct SqshCompressionImplementation sqsh_compression_zstd;
#endif
extern const struct SqshCompressionImplementation sqsh_compression_null;

static const struct SqshCompressionImplementation *
compression_by_id(int id) {
	switch ((enum SqshSuperblockCompressionId)id) {
	case HSQS_COMPRESSION_NONE:
		return &sqsh_compression_null;
#ifdef CONFIG_ZLIB
	case HSQS_COMPRESSION_GZIP:
		return &sqsh_compression_zlib;
#endif
#ifdef CONFIG_LZMA
	case HSQS_COMPRESSION_LZMA:
		return &sqsh_compression_lzma;
#endif
#ifdef CONFIG_XZ
	case HSQS_COMPRESSION_XZ:
		return &sqsh_compression_xz;
#endif
#ifdef CONFIG_LZO
	case HSQS_COMPRESSION_LZO:
		return &sqsh_compression_lzo;
#endif
#ifdef CONFIG_LZ4
	case HSQS_COMPRESSION_LZ4:
		return &sqsh_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
	case HSQS_COMPRESSION_ZSTD:
		return &sqsh_compression_zstd;
#endif
	default:
		return NULL;
	}
}

int
sqsh_compression_init(
		struct SqshCompression *compression, int compression_id,
		size_t block_size) {
	const struct SqshCompressionImplementation *impl =
			compression_by_id(compression_id);
	compression->impl = impl;
	compression->block_size = block_size;
	return 0;
}

int
sqsh_compression_decompress_to_buffer(
		const struct SqshCompression *compression, struct SqshBuffer *buffer,
		const uint8_t *compressed, const size_t compressed_size) {
	int rv = 0;
	const union SqshCompressionOptions *options = NULL;
	const size_t options_size = 0;
	size_t max_size = compression->block_size;
	size_t size = 0;
	uint8_t *decompressed = NULL;
	const struct SqshCompressionImplementation *impl = compression->impl;

	rv = sqsh_buffer_add_capacity(buffer, &decompressed, max_size);
	if (rv < 0) {
		return rv;
	}

	rv = impl->extract(
			options, options_size, decompressed, &max_size, compressed,
			compressed_size);
	if (rv < 0) {
		return rv;
	}
	size = max_size;

	rv = sqsh_buffer_add_size(buffer, size);
	return rv;
}

int
sqsh_compression_cleanup(struct SqshCompression *compression) {
	compression->impl = NULL;
	compression->block_size = 0;
	return 0;
}
