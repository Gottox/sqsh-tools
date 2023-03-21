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
advance_once(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor = {0};
	uint8_t payload[] = {
			SQSH_HEADER, METABLOCK_HEADER(0, 4), 'a', 'b', 'c', 'd',
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(
			&cursor, &sqsh, NULL, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 4);
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == 4);

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_twice(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor = {0};
	uint8_t payload[] = {
			SQSH_HEADER, METABLOCK_HEADER(0, 4), 'a', 'b', 'c',
			'd',         METABLOCK_HEADER(0, 4), 'e', 'f', 'g',
			'h',
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(
			&cursor, &sqsh, NULL, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 4);
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == 4);

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 4, 4);
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == 4);

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "efgh", 4) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_overlapping(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			SQSH_HEADER, METABLOCK_HEADER(0, 4), 'a', 'b', 'c',
			'd',         METABLOCK_HEADER(0, 4), 'e', 'f', 'g',
			'h',
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(
			&cursor, &sqsh, NULL, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 2, 4);
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == 4);

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "cdef", 4) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

DEFINE
TEST(advance_once);
TEST(advance_twice);
TEST(advance_overlapping);
DEFINE_END
