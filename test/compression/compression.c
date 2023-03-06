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
 * @file         buffer.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_compression_private.h"
#include <stdint.h>

static void
decompress_test(
		const struct SqshCompressionImpl *impl, uint8_t *input,
		size_t input_size) {
	int rv;
	uint8_t output[16];
	size_t output_size = sizeof(output);
	sqsh__compression_context_t context = {0};

	assert(impl != NULL);

	rv = impl->init(context, output, output_size);
	assert(rv >= 0);
	rv = impl->decompress(context, input, input_size);
	assert(rv >= 0);
	rv = impl->finish(context, output, &output_size);
	assert(rv >= 0);

	assert(output_size == 4);
	assert(memcmp(output, "abcd", 4) == 0);
}

static void
decompress_test_split(
		const struct SqshCompressionImpl *impl, uint8_t *input,
		size_t input_size) {
	int rv;
	uint8_t output[16];
	size_t output_size = sizeof(output);
	sqsh__compression_context_t context = {0};

	assert(impl != NULL);

	for (sqsh_index_t offset = 0; offset < input_size; offset++) {
		memset(output, 0, sizeof(output));

		rv = impl->init(context, output, output_size);
		assert(rv >= 0);
		rv = impl->decompress(context, input, offset);
		assert(rv >= 0);
		rv = impl->decompress(context, &input[offset], input_size - offset);
		assert(rv >= 0);
		rv = impl->finish(context, output, &output_size);
		assert(rv >= 0);

		assert(output_size == 4);
		assert(memcmp(output, "abcd", 4) == 0);
	}
}

static void
decompress_lzma(void) {
	uint8_t input[] = {0x5d, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0xff, 0xff,
					   0xff, 0xff, 0xff, 0xff, 0x00, 0x30, 0x98, 0x88, 0x98,
					   0x46, 0x7e, 0x1e, 0xb2, 0xff, 0xfa, 0x1c, 0x80, 0x00};

	decompress_test(sqsh__impl_lzma, input, sizeof(input));
}

static void
decompress_lzma_split(void) {
	uint8_t input[] = {0x5d, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0xff, 0xff,
					   0xff, 0xff, 0xff, 0xff, 0x00, 0x30, 0x98, 0x88, 0x98,
					   0x46, 0x7e, 0x1e, 0xb2, 0xff, 0xfa, 0x1c, 0x80, 0x00};

	decompress_test_split(sqsh__impl_lzma, input, sizeof(input));
}

static void
decompress_xz(void) {
	uint8_t input[] = {0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6,
					   0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01, 0x16, 0x00,
					   0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0x01, 0x00, 0x03,
					   0x61, 0x62, 0x63, 0x64, 0x00, 0xba, 0x60, 0x59, 0x6e,
					   0x59, 0x28, 0x9d, 0x3c, 0x00, 0x01, 0x1c, 0x04, 0x6f,
					   0x2c, 0x9c, 0xc1, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00,
					   0x00, 0x00, 0x00, 0x04, 0x59, 0x5a};

	decompress_test(sqsh__impl_xz, input, sizeof(input));
}

static void
decompress_xz_split(void) {
	uint8_t input[] = {0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6,
					   0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01, 0x16, 0x00,
					   0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0x01, 0x00, 0x03,
					   0x61, 0x62, 0x63, 0x64, 0x00, 0xba, 0x60, 0x59, 0x6e,
					   0x59, 0x28, 0x9d, 0x3c, 0x00, 0x01, 0x1c, 0x04, 0x6f,
					   0x2c, 0x9c, 0xc1, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00,
					   0x00, 0x00, 0x00, 0x04, 0x59, 0x5a};

	decompress_test_split(sqsh__impl_xz, input, sizeof(input));
}

static void
decompress_lz4(void) {
	uint8_t input[] = {0x40, 0x61, 0x62, 0x63, 0x64};

	decompress_test(sqsh__impl_lz4, input, sizeof(input));
}

static void
decompress_lz4_split(void) {
	uint8_t input[] = {0x40, 0x61, 0x62, 0x63, 0x64};

	decompress_test_split(sqsh__impl_lz4, input, sizeof(input));
}

static void
decompress_lzo(void) {
	uint8_t input[] = {0x15, 0x61, 0x62, 0x63, 0x64, 0x11, 0x00, 0x00};

	decompress_test(sqsh__impl_lzo, input, sizeof(input));
}

static void
decompress_lzo_split(void) {
	uint8_t input[] = {0x15, 0x61, 0x62, 0x63, 0x64, 0x11, 0x00, 0x00};

	decompress_test_split(sqsh__impl_lzo, input, sizeof(input));
}

static void
decompress_zlib(void) {
	uint8_t input[] = {
			ZLIB_ABCD,
	};

	decompress_test(sqsh__impl_zlib, input, sizeof(input));
}

static void
decompress_zlib_split(void) {
	uint8_t input[] = {
			ZLIB_ABCD,
	};

	decompress_test_split(sqsh__impl_zlib, input, sizeof(input));
}

static void
decompress_zstd(void) {
	uint8_t input[] = {0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x04, 0x21,
					   0x00, 0x00, 0x61, 0x62, 0x63, 0x64};

	decompress_test(sqsh__impl_zstd, input, sizeof(input));
}

static void
decompress_zstd_split(void) {
	uint8_t input[] = {0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x04, 0x21,
					   0x00, 0x00, 0x61, 0x62, 0x63, 0x64};

	decompress_test_split(sqsh__impl_zstd, input, sizeof(input));
}

static void *
multithreaded_lzo_worker(void *arg) {
	(void)arg;
	uint8_t input[] = {0x15, 0x61, 0x62, 0x63, 0x64, 0x11, 0x00, 0x00};

	for (int i = 0; i < 10000; i++) {
		decompress_test(sqsh__impl_lzo, input, sizeof(input));
	}
	return 0;
}

static void
multithreaded_lzo(void) {
	(void)multithreaded_lzo_worker;
	int rv;
	pthread_t threads[16] = {0};

	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_create(&threads[i], NULL, multithreaded_lzo_worker, NULL);
		assert(rv == 0);
	}

	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_join(threads[i], NULL);
		assert(rv == 0);
	}
}

DEFINE
TEST(decompress_lzma);
TEST_OFF(decompress_lzma_split);
TEST(decompress_xz);
TEST(decompress_xz_split);
TEST(decompress_lz4);
TEST_OFF(decompress_lz4_split);
TEST_OFF(decompress_lzo);
TEST_OFF(decompress_lzo_split);
TEST(decompress_zlib);
TEST_OFF(decompress_zlib_split);
TEST(decompress_zstd);
TEST(decompress_zstd_split);
TEST_OFF(multithreaded_lzo);
DEFINE_END
