/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2023, Enno Boland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         compression_options.c
 */

#include "../common.h"
#include <testlib.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>

#define GZIP_OPTIONS(level, winsize, strategy) \
	UINT32_BYTES(level), UINT16_BYTES(winsize), UINT16_BYTES(strategy)
#define XZ_OPTIONS(dictionary_size, filters) \
	UINT32_BYTES(dictionary_size), UINT32_BYTES(filters)
#define LZ4_OPTIONS(version, flags) UINT32_BYTES(version), UINT32_BYTES(flags)
#define ZSTD_OPTIONS(level) UINT32_BYTES(level)
#define LZO_OPTIONS(algorithm, level) \
	UINT32_BYTES(algorithm), UINT32_BYTES(level)

// Make sure, that lzo compression is not NULL, otherwise libsqsh will refuse
// to init the archive.
const struct SqshExtractorImpl *const sqsh__impl_lzo = (void *)0x1;

static void
load_compression_options_gzip(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(GZIP_OPTIONS(0, 0, 0))),
			GZIP_OPTIONS(123, 456, 789)};
	payload[20] = 1;
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_size(options) == 8);

	assert(sqsh_compression_options_gzip_compression_level(options) == 123);
	assert(sqsh_compression_options_gzip_window_size(options) == 456);
	assert(sqsh_compression_options_gzip_strategies(options) == 789);

	assert(sqsh_compression_options_xz_dictionary_size(options) == UINT32_MAX);
	assert(sqsh_compression_options_xz_filters(options) == UINT32_MAX);

	assert(sqsh_compression_options_lz4_version(options) == UINT32_MAX);
	assert(sqsh_compression_options_lz4_flags(options) == UINT32_MAX);

	assert(sqsh_compression_options_zstd_compression_level(options) ==
		   UINT32_MAX);

	assert(sqsh_compression_options_lzo_algorithm(options) == UINT32_MAX);
	assert(sqsh_compression_options_lzo_compression_level(options) ==
		   UINT32_MAX);
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

static void
load_compression_options_xz(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(XZ_OPTIONS(0, 0))),
			XZ_OPTIONS(123, 456)};
	payload[20] = 4;
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_size(options) == 8);

	assert(sqsh_compression_options_gzip_compression_level(options) ==
		   UINT32_MAX);
	assert(sqsh_compression_options_gzip_window_size(options) == UINT16_MAX);
	assert(sqsh_compression_options_gzip_strategies(options) == UINT16_MAX);

	assert(sqsh_compression_options_xz_dictionary_size(options) == 123);
	assert(sqsh_compression_options_xz_filters(options) == 456);

	assert(sqsh_compression_options_lz4_version(options) == UINT32_MAX);
	assert(sqsh_compression_options_lz4_flags(options) == UINT32_MAX);

	assert(sqsh_compression_options_zstd_compression_level(options) ==
		   UINT32_MAX);

	assert(sqsh_compression_options_lzo_algorithm(options) == UINT32_MAX);
	assert(sqsh_compression_options_lzo_compression_level(options) ==
		   UINT32_MAX);
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

static void
load_compression_options_lz4(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(LZ4_OPTIONS(0, 0))),
			LZ4_OPTIONS(123, 456)};
	payload[20] = 5;
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_size(options) == 8);

	assert(sqsh_compression_options_gzip_compression_level(options) ==
		   UINT32_MAX);
	assert(sqsh_compression_options_gzip_window_size(options) == UINT16_MAX);
	assert(sqsh_compression_options_gzip_strategies(options) == UINT16_MAX);

	assert(sqsh_compression_options_xz_dictionary_size(options) == UINT32_MAX);
	assert(sqsh_compression_options_xz_filters(options) == UINT32_MAX);

	assert(sqsh_compression_options_lz4_version(options) == 123);
	assert(sqsh_compression_options_lz4_flags(options) == 456);

	assert(sqsh_compression_options_zstd_compression_level(options) ==
		   UINT32_MAX);

	assert(sqsh_compression_options_lzo_algorithm(options) == UINT32_MAX);
	assert(sqsh_compression_options_lzo_compression_level(options) ==
		   UINT32_MAX);
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

static void
load_compression_options_zstd(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(ZSTD_OPTIONS(0))),
			ZSTD_OPTIONS(123)};
	payload[20] = 6;
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_size(options) == 4);

	assert(sqsh_compression_options_gzip_compression_level(options) ==
		   UINT32_MAX);
	assert(sqsh_compression_options_gzip_window_size(options) == UINT16_MAX);
	assert(sqsh_compression_options_gzip_strategies(options) == UINT16_MAX);

	assert(sqsh_compression_options_xz_dictionary_size(options) == UINT32_MAX);
	assert(sqsh_compression_options_xz_filters(options) == UINT32_MAX);

	assert(sqsh_compression_options_lz4_version(options) == UINT32_MAX);
	assert(sqsh_compression_options_lz4_flags(options) == UINT32_MAX);

	assert(sqsh_compression_options_zstd_compression_level(options) == 123);

	assert(sqsh_compression_options_lzo_algorithm(options) == UINT32_MAX);
	assert(sqsh_compression_options_lzo_compression_level(options) ==
		   UINT32_MAX);
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

static void
load_compression_options_lzo(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(LZO_OPTIONS(0, 0))),
			LZO_OPTIONS(123, 456)};
	payload[20] = 3;
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_size(options) == 8);

	assert(sqsh_compression_options_gzip_compression_level(options) ==
		   UINT32_MAX);
	assert(sqsh_compression_options_gzip_compression_level(options) ==
		   UINT32_MAX);
	assert(sqsh_compression_options_gzip_window_size(options) == UINT16_MAX);
	assert(sqsh_compression_options_gzip_strategies(options) == UINT16_MAX);

	assert(sqsh_compression_options_xz_dictionary_size(options) == UINT32_MAX);
	assert(sqsh_compression_options_xz_filters(options) == UINT32_MAX);

	assert(sqsh_compression_options_lz4_version(options) == UINT32_MAX);
	assert(sqsh_compression_options_lz4_flags(options) == UINT32_MAX);

	assert(sqsh_compression_options_zstd_compression_level(options) ==
		   UINT32_MAX);

	assert(sqsh_compression_options_lzo_algorithm(options) == 123);
	assert(sqsh_compression_options_lzo_compression_level(options) == 456);
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(load_compression_options_gzip)
TEST(load_compression_options_xz)
TEST(load_compression_options_lz4)
TEST(load_compression_options_zstd)
TEST(load_compression_options_lzo)
END_TESTS
