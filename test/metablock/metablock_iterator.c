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
 * @file         metablock.c
 */

#include "../common.h"
#include "../test.h"

#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_metablock_private.h>
#include <stdint.h>

static void
next_once(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {
			SQSH_HEADER, METABLOCK_HEADER(0, 4), 'a', 'b', 'c', 'd',
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv == 0);

	assert(sqsh__metablock_iterator_size(&iter) == 4);

	p = sqsh__metablock_iterator_data(&iter);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
next_failing_with_no_compression(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {
			SQSH_HEADER, METABLOCK_HEADER(1, 4), 'a', 'b', 'c', 'd',
	};
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv != 0);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
next_twice(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {
			SQSH_HEADER, METABLOCK_HEADER(0, 4), 'a', 'b', 'c',
			'd',         METABLOCK_HEADER(0, 4), 'e', 'f', 'g',
			'h',
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv == 0);

	assert(sqsh__metablock_iterator_size(&iter) == 4);

	p = sqsh__metablock_iterator_data(&iter);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv == 0);

	assert(sqsh__metablock_iterator_size(&iter) == 4);

	p = sqsh__metablock_iterator_data(&iter);
	assert(p != NULL);
	assert(memcmp(p, "efgh", 4) == 0);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
next_compressed(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {
			SQSH_HEADER, METABLOCK_HEADER(1, CHUNK_SIZE(ZLIB_ABCD)),
			ZLIB_ABCD,   METABLOCK_HEADER(1, CHUNK_SIZE(ZLIB_EFGH)),
			ZLIB_EFGH,
	};
	const uint8_t *p;

	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv == 0);

	assert(sqsh__metablock_iterator_size(&iter) == 4);

	p = sqsh__metablock_iterator_data(&iter);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_iterator_next(&iter);
	assert(rv == 0);

	assert(sqsh__metablock_iterator_size(&iter) == 4);

	p = sqsh__metablock_iterator_data(&iter);
	assert(p != NULL);
	assert(memcmp(p, "efgh", 4) == 0);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

DEFINE
TEST(next_once);
TEST(next_failing_with_no_compression);
TEST(next_twice);
TEST(next_compressed);
DEFINE_END
