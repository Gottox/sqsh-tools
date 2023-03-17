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
 * @file         compression_manager.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_compression_private.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_mapper_private.h"

static void
decompress(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshCompression compression = {0};
	struct SqshCompressionManager manager = {0};
	const struct SqshBuffer *buffer = NULL;
	uint8_t payload[] = {ZLIB_ABCD};
	size_t target_size;

	rv = sqsh__compression_init(&compression, SQSH_COMPRESSION_GZIP, 4096);
	uint8_t *data = mk_stub(&archive, payload, sizeof(payload), &target_size);
	assert(rv == 0);

	rv = sqsh__compression_manager_init(
			&manager, &archive, &compression, 0, target_size, 0);
	assert(rv == 0);

	rv = sqsh__compression_manager_get(
			&manager, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload), &buffer);
	assert(rv == 0);
	assert(buffer != NULL);
	assert(sqsh__buffer_size(buffer) == 4);
	assert(memcmp(sqsh__buffer_data(buffer), "abcd", 4) == 0);

	sqsh__compression_manager_release(&manager, buffer);
	sqsh__compression_manager_cleanup(&manager);
	sqsh__compression_cleanup(&compression);
	sqsh__archive_cleanup(&archive);
	free(data);
}

DEFINE
TEST(decompress);
DEFINE_END
