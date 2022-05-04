/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 12:16:28 CEST
 */

#include "../compression/compression.h"
#include "../context/superblock_context.h"
#include "../data/metablock.h"
#include "../data/superblock_internal.h"
#include "../error.h"
#include "../utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
hsqs_buffer_init(
		struct HsqsBuffer *buffer, int compression_id, int block_size) {
	int rv = 0;
	const struct HsqsCompressionImplementation *impl;

	impl = compression_by_id(compression_id);
	if (impl == NULL) {
		return -HSQS_ERROR_COMPRESSION_UNKNOWN;
	}

	if (rv < 0) {
		return rv;
	}
	buffer->impl = impl;
	buffer->block_size = block_size;
	buffer->data = NULL;
	buffer->size = 0;

	return rv;
}

int
hsqs_buffer_append(
		struct HsqsBuffer *buffer, const uint8_t *source,
		const size_t source_size) {
	int rv = 0;
	const size_t buffer_size = buffer->size;
	size_t target_size;
	size_t new_size;

	if (ADD_OVERFLOW(buffer_size, source_size, &new_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	buffer->data = realloc(buffer->data, new_size);
	if (buffer->data == NULL) {
		return -HSQS_ERROR_MALLOC_FAILED;
	}

	target_size = source_size;
	rv = hsqs_compression_null.extract(
			NULL, 0, &buffer->data[buffer_size], &target_size, source,
			source_size);
	if (rv < 0)
		return rv;

	buffer->size += target_size;

	return rv;
}
int
hsqs_buffer_append_block(
		struct HsqsBuffer *buffer, const uint8_t *source,
		const size_t source_size, bool is_compressed) {
	const union HsqsCompressionOptions *options = NULL;
	size_t options_size = 0;
	// const struct HsqsCompressionOptionsContext *options_context;
	const struct HsqsCompressionImplementation *impl =
			is_compressed ? buffer->impl : &hsqs_compression_null;
	int rv = 0;
	size_t block_size = buffer->block_size;
	const size_t buffer_size = buffer->size;
	size_t new_size;

	if (ADD_OVERFLOW(buffer_size, block_size, &new_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	buffer->data = realloc(buffer->data, new_size);
	if (buffer->data == NULL) {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
	// if (is_compressed) {
	//	options_context = NULL;
	//			//hsqs_superblock_compression_options(buffer->superblock);
	//	if (options_context) {
	//		options = hsqs_compression_options(options_context);
	//		options_size = hsqs_compression_options_size(options_context);
	//	}
	//}

	rv = impl->extract(
			options, options_size, &buffer->data[buffer_size], &block_size,
			source, source_size);
	if (rv < 0)
		return rv;

	buffer->size += block_size;

	return rv;
}

const uint8_t *
hsqs_buffer_data(const struct HsqsBuffer *buffer) {
	return buffer->data;
}
size_t
hsqs_buffer_size(const struct HsqsBuffer *buffer) {
	return buffer->size;
}

int
hsqs_buffer_cleanup(struct HsqsBuffer *buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->size = 0;
	return 0;
}
