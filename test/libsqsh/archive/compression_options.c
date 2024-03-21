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
#include <utest.h>

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
const struct SqshExtractorImpl *const volatile sqsh__impl_lzo = (void *)0x1;

UTEST(compression_options, load_compression_options_gzip) {
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
	ASSERT_EQ(rv, 0);
	ASSERT_NE(options, NULL);

	ASSERT_EQ((size_t)8, sqsh_compression_options_size(options));

	ASSERT_EQ(
			(uint32_t)123,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(
			(uint16_t)456, sqsh_compression_options_gzip_window_size(options));
	ASSERT_EQ(
			(enum SqshGzipStrategies)789,
			sqsh_compression_options_gzip_strategies(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_dictionary_size(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_filters(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_version(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_flags(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_zstd_compression_level(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lzo_algorithm(options));
	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_lzo_compression_level(options));
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

UTEST(compression_options, load_compression_options_xz) {
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
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, options);

	ASSERT_EQ((size_t)8, sqsh_compression_options_size(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(UINT16_MAX, sqsh_compression_options_gzip_window_size(options));
	ASSERT_EQ(
			(enum SqshGzipStrategies)UINT16_MAX,
			sqsh_compression_options_gzip_strategies(options));

	ASSERT_EQ(
			(uint32_t)123,
			sqsh_compression_options_xz_dictionary_size(options));
	ASSERT_EQ(
			(enum SqshXzFilters)456,
			sqsh_compression_options_xz_filters(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_version(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_flags(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_zstd_compression_level(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lzo_algorithm(options));
	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_lzo_compression_level(options));
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

UTEST(compression_options, load_compression_options_lz4) {
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
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, options);

	ASSERT_EQ((size_t)8, sqsh_compression_options_size(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(UINT16_MAX, sqsh_compression_options_gzip_window_size(options));
	ASSERT_EQ(
			(enum SqshGzipStrategies)UINT16_MAX,
			sqsh_compression_options_gzip_strategies(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_dictionary_size(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_filters(options));

	ASSERT_EQ((uint32_t)123, sqsh_compression_options_lz4_version(options));
	ASSERT_EQ((uint32_t)456, sqsh_compression_options_lz4_flags(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_zstd_compression_level(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lzo_algorithm(options));
	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_lzo_compression_level(options));
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

UTEST(compression_options, load_compression_options_zstd) {
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
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, options);

	ASSERT_EQ((size_t)4, sqsh_compression_options_size(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(UINT16_MAX, sqsh_compression_options_gzip_window_size(options));
	ASSERT_EQ(
			(enum SqshGzipStrategies)UINT16_MAX,
			sqsh_compression_options_gzip_strategies(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_dictionary_size(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_filters(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_version(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_flags(options));

	ASSERT_EQ(
			(uint32_t)123,
			sqsh_compression_options_zstd_compression_level(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lzo_algorithm(options));
	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_lzo_compression_level(options));
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

UTEST(compression_options, load_compression_options_lzo) {
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
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, options);

	ASSERT_EQ((size_t)8, sqsh_compression_options_size(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_gzip_compression_level(options));
	ASSERT_EQ(UINT16_MAX, sqsh_compression_options_gzip_window_size(options));
	ASSERT_EQ(
			(enum SqshGzipStrategies)UINT16_MAX,
			sqsh_compression_options_gzip_strategies(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_dictionary_size(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_xz_filters(options));

	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_version(options));
	ASSERT_EQ(UINT32_MAX, sqsh_compression_options_lz4_flags(options));

	ASSERT_EQ(
			UINT32_MAX,
			sqsh_compression_options_zstd_compression_level(options));

	ASSERT_EQ(
			(enum SqshLzoAlgorithm)123,
			sqsh_compression_options_lzo_algorithm(options));
	ASSERT_EQ(
			(uint32_t)456,
			sqsh_compression_options_lzo_compression_level(options));
	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

UTEST_MAIN()
