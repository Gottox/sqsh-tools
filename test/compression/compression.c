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

#include <sqsh_compression_private.h>
#include <stdint.h>

static void
decompress_test(
		const struct SqshCompressionImpl *impl, uint8_t *input,
		size_t input_size) {
	int rv;
	uint8_t output[16];
	size_t output_size = sizeof(output);
	sqsh__compression_context_t context = {0};

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
decompress_lzma(void) {
#ifdef CONFIG_LZMA
	uint8_t input[] = {0x5d, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0xff, 0xff,
					   0xff, 0xff, 0xff, 0xff, 0x00, 0x30, 0x98, 0x88, 0x98,
					   0x46, 0x7e, 0x1e, 0xb2, 0xff, 0xfa, 0x1c, 0x80, 0x00};

	decompress_test(sqsh__lzma_impl, input, sizeof(input));
#endif
}

static void
decompress_xz(void) {
#ifdef CONFIG_LZMA
	uint8_t input[] = {0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6,
					   0xd6, 0xb4, 0x46, 0x02, 0x00, 0x21, 0x01, 0x16, 0x00,
					   0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3, 0x01, 0x00, 0x03,
					   0x61, 0x62, 0x63, 0x64, 0x00, 0xba, 0x60, 0x59, 0x6e,
					   0x59, 0x28, 0x9d, 0x3c, 0x00, 0x01, 0x1c, 0x04, 0x6f,
					   0x2c, 0x9c, 0xc1, 0x1f, 0xb6, 0xf3, 0x7d, 0x01, 0x00,
					   0x00, 0x00, 0x00, 0x04, 0x59, 0x5a};

	decompress_test(sqsh__xz_impl, input, sizeof(input));
#endif
}

static void
decompress_lz4(void) {
#ifdef CONFIG_LZ4
	uint8_t input[] = {0x40, 0x61, 0x62, 0x63, 0x64};

	decompress_test(sqsh__lz4_impl, input, sizeof(input));
#endif
}

static void
decompress_lzo(void) {
#ifdef CONFIG_LZO
	uint8_t input[] = {0x15, 0x61, 0x62, 0x63, 0x64, 0x11, 0x00, 0x00};

	decompress_test(sqsh__lzo_impl, input, sizeof(input));
#endif
}

static void
decompress_zlib(void) {
#ifdef CONFIG_ZLIB
	uint8_t input[] = {
			ZLIB_ABCD,
	};

	decompress_test(sqsh__zlib_impl, input, sizeof(input));
#endif
}

static void
decompress_zstd(void) {
	uint8_t input[] = {0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x04, 0x21,
					   0x00, 0x00, 0x61, 0x62, 0x63, 0x64};

	decompress_test(sqsh__zstd_impl, input, sizeof(input));
}

static void *
multithreaded_lzo_worker(void *arg) {
	(void)arg;
	uint8_t input[] = {0x15, 0x61, 0x62, 0x63, 0x64, 0x11, 0x00, 0x00};

	for (int i = 0; i < 10000; i++) {
		decompress_test(sqsh__lzo_impl, input, sizeof(input));
	}
	return 0;
}
static void
multithreaded_lzo(void) {
#ifdef CONFIG_LZO
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
#endif
}

DEFINE
TEST(decompress_lzma);
TEST(decompress_xz);
TEST(decompress_lz4);
TEST(multithreaded_lzo);
TEST(decompress_lzo);
TEST(decompress_zlib);
TEST(decompress_zstd);
DEFINE_END
