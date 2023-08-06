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

#include "../../include/sqsh_archive_private.h"
#include "../../lib/utils/utils.h"

static void
load_compression_options(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			METABLOCK_HEADER(0, CHUNK_SIZE(GZIP_OPTIONS(0, 0, 0))),
			GZIP_OPTIONS(123, 456, 789)};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshCompressionOptions *options =
			sqsh_compression_options_new(&archive, &rv);
	assert(rv == 0);
	assert(options != NULL);

	assert(sqsh_compression_options_gzip_compression_level(options) == 123);
	assert(sqsh_compression_options_gzip_window_size(options) == 456);
	assert(sqsh_compression_options_gzip_strategies(options) == 789);

	sqsh_compression_options_free(options);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(load_compression_options)
END_TESTS
