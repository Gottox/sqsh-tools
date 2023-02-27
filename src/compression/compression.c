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

#ifdef CONFIG_LZMA
int sqsh_extract_lzma(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
int sqsh_extract_xz(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
#else
const sqsh_extract_func_t sqsh_extract_lzma = NULL;
const sqsh_extract_func_t sqsh_extract_xz = NULL;
#endif

#ifdef CONFIG_LZ4
int sqsh_extract_lz4(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
#else
const sqsh_extract_func_t sqsh_extract_lz4 = NULL;
#endif

#ifdef CONFIG_LZO
int sqsh_extract_lzo2(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
#else
const sqsh_extract_func_t sqsh_extract_lzo2 = NULL;
#endif

#ifdef CONFIG_ZLIB
int sqsh_extract_zlib(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
#else
const sqsh_extract_func_t sqsh_extract_zlib = NULL;
#endif

#ifdef CONFIG_ZSTD
int sqsh_extract_zstd(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);
#else
const sqsh_extract_func_t sqsh_extract_zstd = NULL;
#endif

int sqsh_extract_null(
		uint8_t *target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size);

sqsh_extract_func_t
compression_by_id(int id) {
	switch ((enum SqshSuperblockCompressionId)id) {
	case SQSH_COMPRESSION_GZIP:
		return sqsh_extract_zlib;
	case SQSH_COMPRESSION_LZMA:
		return sqsh_extract_lzma;
	case SQSH_COMPRESSION_XZ:
		return sqsh_extract_xz;
	case SQSH_COMPRESSION_LZO:
		return sqsh_extract_lzo2;
	case SQSH_COMPRESSION_LZ4:
		return sqsh_extract_lz4;
	case SQSH_COMPRESSION_ZSTD:
		return sqsh_extract_zstd;
	default:
		return NULL;
	}
}

int
sqsh__compression_init(
		struct SqshCompression *compression, int compression_id,
		size_t block_size) {
	sqsh_extract_func_t impl = compression_by_id(compression_id);
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
	sqsh_extract_func_t extract = compression->impl;

	rv = sqsh__buffer_add_capacity(buffer, &decompressed, max_size);
	if (rv < 0) {
		return rv;
	}

	rv = extract(decompressed, &max_size, compressed, compressed_size);
	if (rv < 0) {
		return rv;
	}
	size = max_size;

	rv = sqsh__buffer_add_size(buffer, size);
	return rv;
}

int
sqsh__compression_cleanup(struct SqshCompression *compression) {
	compression->impl = NULL;
	compression->block_size = 0;
	return 0;
}
