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

#include "../hsqs.h"
//#include "../data/compression_options.h"
#include "../context/superblock_context.h"
#include "../error.h"
#include "compression.h"
#include "data/superblock.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef CONFIG_ZLIB
extern const struct HsqsCompressionImplementation hsqs_compression_zlib;
#endif
#ifdef CONFIG_LZMA
extern const struct HsqsCompressionImplementation hsqs_compression_lzma;
extern const struct HsqsCompressionImplementation hsqs_compression_xz;
#endif
#ifdef CONFIG_LZO
extern const struct HsqsCompressionImplementation hsqs_compression_lzo;
#endif
#ifdef CONFIG_LZ4
extern const struct HsqsCompressionImplementation hsqs_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
extern const struct HsqsCompressionImplementation hsqs_compression_zstd;
#endif
extern const struct HsqsCompressionImplementation hsqs_compression_null;

static const struct HsqsCompressionImplementation *
compression_by_id(int id) {
	switch ((enum HsqsSuperblockCompressionId)id) {
	case HSQS_COMPRESSION_NONE:
		return &hsqs_compression_null;
#ifdef CONFIG_ZLIB
	case HSQS_COMPRESSION_GZIP:
		return &hsqs_compression_zlib;
#endif
#ifdef CONFIG_LZMA
	case HSQS_COMPRESSION_LZMA:
		return &hsqs_compression_lzma;
#endif
#ifdef CONFIG_XZ
	case HSQS_COMPRESSION_XZ:
		return &hsqs_compression_xz;
#endif
#ifdef CONFIG_LZO
	case HSQS_COMPRESSION_LZO:
		return &hsqs_compression_lzo;
#endif
#ifdef CONFIG_LZ4
	case HSQS_COMPRESSION_LZ4:
		return &hsqs_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
	case HSQS_COMPRESSION_ZSTD:
		return &hsqs_compression_zstd;
#endif
	default:
		return NULL;
	}
}

int
hsqs_compression_init(
		struct HsqsCompression *compression, int compression_id,
		size_t block_size) {
	const struct HsqsCompressionImplementation *impl =
			compression_by_id(compression_id);
	compression->impl = impl;
	compression->block_size = block_size;
	return 0;
}

int
hsqs_compression_decompress_to_buffer(
		const struct HsqsCompression *compression, struct HsqsBuffer *buffer,
		const uint8_t *compressed, const size_t compressed_size) {
	int rv = 0;
	const union HsqsCompressionOptions *options = NULL;
	const size_t options_size = 0;
	size_t max_size = compression->block_size;
	size_t size = 0;
	uint8_t *decompressed = NULL;
	const struct HsqsCompressionImplementation *impl = compression->impl;

	rv = hsqs_buffer_add_capacity(buffer, &decompressed, max_size);
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

	rv = hsqs_buffer_add_size(buffer, size);
	return rv;
}

int
hsqs_compression_cleanup(struct HsqsCompression *compression) {
	compression->impl = NULL;
	compression->block_size = 0;
	return 0;
}
