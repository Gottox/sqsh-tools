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
 * @file         compression_options_data.c
 */

#define _DEFAULT_SOURCE

#include "../../include/sqsh_data_private.h"

#include <endian.h>

struct SQSH_UNALIGNED SqshDataCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsGzip) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_GZIP);

struct SQSH_UNALIGNED SqshDataCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsXz) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ);

struct SQSH_UNALIGNED SqshDataCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsLz4) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZ4);

struct SQSH_UNALIGNED SqshDataCompressionOptionsZstd {
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsZstd) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_ZSTD);

struct SQSH_UNALIGNED SqshDataCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataCompressionOptionsLzo) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS_LZO);

union SqshDataCompressionOptions {
	struct SqshDataCompressionOptionsGzip gzip;
	struct SqshDataCompressionOptionsXz xz;
	struct SqshDataCompressionOptionsLz4 lz4;
	struct SqshDataCompressionOptionsZstd zstd;
	struct SqshDataCompressionOptionsLzo lzo;
};
SQSH_STATIC_ASSERT(
		sizeof(union SqshDataCompressionOptions) ==
		SQSH_SIZEOF_COMPRESSION_OPTIONS);

uint32_t
sqsh_compression_data_options_gzip_compression_level(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->gzip.compression_level);
}
uint16_t
sqsh_compression_data_options_gzip_window_size(
		const union SqshDataCompressionOptions *options) {
	return le16toh(options->gzip.window_size);
}
uint16_t
sqsh_compression_data_options_gzip_strategies(
		const union SqshDataCompressionOptions *options) {
	return le16toh(options->gzip.strategies);
}

uint32_t
sqsh_compression_data_options_xz_dictionary_size(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->xz.dictionary_size);
}
uint32_t
sqsh_compression_data_options_xz_filters(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->xz.filters);
}

uint32_t
sqsh_compression_data_options_lz4_version(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->lz4.version);
}
uint32_t
sqsh_compression_data_options_lz4_flags(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->lz4.flags);
}

uint32_t
sqsh_compression_data_options_zstd_compression_level(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->zstd.compression_level);
}

uint32_t
sqsh_compression_data_options_lzo_algorithm(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->lzo.algorithm);
}
uint32_t
sqsh_compression_data_options_lzo_compression_level(
		const union SqshDataCompressionOptions *options) {
	return le32toh(options->lzo.compression_level);
}
