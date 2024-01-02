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
 * @file         metablock_reader.c
 */

#include "../common.h"
#include <testlib.h>

#include <mksqsh_metablock.h>
#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_metablock_private.h>
#include <stdint.h>

static void
advance_once(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor = {0};
	uint8_t payload[8192] = {0};
	const uint8_t *p;

	FILE *farchive = test_sqsh_prepare_archive(payload, sizeof(payload));

	struct MksqshMetablock metablock_writer = {0};
	mksqsh__metablock_init(&metablock_writer, farchive);
	mksqsh__metablock_write(&metablock_writer, (const uint8_t *)"abcd", 4);
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_cleanup(&metablock_writer);

	test_sqsh_init_archive(&sqsh, farchive, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(
			&cursor, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
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
	uint8_t payload[16384] = {0};
	uint8_t metablock_content[] = {
			'a', 'b', 'c', 'd', [8192] = 'e', 'f', 'g', 'h',
	};
	const uint8_t *p;

	FILE *farchive = test_sqsh_prepare_archive(payload, sizeof(payload));

	fseek(farchive, 1024, SEEK_SET);
	struct MksqshMetablock metablock_writer = {0};
	mksqsh__metablock_init(&metablock_writer, farchive);
	mksqsh__metablock_write(
			&metablock_writer, metablock_content, sizeof(metablock_content));
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_cleanup(&metablock_writer);

	test_sqsh_init_archive(&sqsh, farchive, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 4);
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == 4);

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8192, 4);
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
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			'a', 'b', 'c', 'd',
			[1024 + sizeof(struct SqshDataMetablock) + 8192] = METABLOCK_HEADER(0, 8192),
			'e', 'f', 'g', 'h',
			[1024 + 2*(sizeof(struct SqshDataMetablock) + 8192)-1] = 0,
			/* clang-format on */
	};
	uint8_t metablock_content[] = {
			/* clang-format off */
			'a', 'b', 'c', 'd',
			[8192] = 'e', 'f', 'g', 'h',
			/* clang-format on */
	};
	const uint8_t *p;

	FILE *farchive = test_sqsh_prepare_archive(payload, sizeof(payload));

	fseek(farchive, 1024, SEEK_SET);
	struct MksqshMetablock metablock_writer = {0};
	mksqsh__metablock_init(&metablock_writer, farchive);
	mksqsh__metablock_write(
			&metablock_writer, metablock_content, sizeof(metablock_content));
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_cleanup(&metablock_writer);

	test_sqsh_init_archive(&sqsh, farchive, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, sizeof(metablock_content));
	assert(rv == 0);

	assert(sqsh__metablock_reader_size(&cursor) == sizeof(metablock_content));

	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, metablock_content, sizeof(metablock_content)) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_overlapping_2(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			[1024 + sizeof(struct SqshDataMetablock) + 8192 - 4] = 'a', 'b', 'c', 'd',
			[1024 + sizeof(struct SqshDataMetablock) + 8192] = METABLOCK_HEADER(0, 8192),
			'e', 'f', 'g', 'h',
			[1024 + 2*(sizeof(struct SqshDataMetablock) + 8192)-1] = 0,
			/* clang-format on */
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8192 - 4, 3);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 3);
	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abc", 3) == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 8);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 8);
	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abcdefgh", 8) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_overlapping_3(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			[1024 + sizeof(struct SqshDataMetablock)+8180] =
			1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			[1024 + sizeof(struct SqshDataMetablock) + 8192] = METABLOCK_HEADER(0, 32 - 12),
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
			/* clang-format on */
	};
	uint8_t expected[] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
						  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
						  23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
	const uint8_t *data;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8180, 16);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 16);
	data = sqsh__metablock_reader_data(&cursor);
	assert(data != NULL);
	assert(memcmp(data, expected, 16) == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 32);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 32);
	data = sqsh__metablock_reader_data(&cursor);
	assert(data != NULL);
	assert(memcmp(data, expected, 32) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_skip(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			[1024 + sizeof(struct SqshDataMetablock)] = 'a', 'b', 'c', 'd',
			[1024 + sizeof(struct SqshDataMetablock) + 8192] = METABLOCK_HEADER(0, 4),
			'e', 'f', 'g', 'h',
			/* clang-format on */
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8192, 4);
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
advance_skip_2(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			[1024 + sizeof(struct SqshDataMetablock)] = 'a', 'b', 'c', 'd',
			[1024 + sizeof(struct SqshDataMetablock) + 8192] = METABLOCK_HEADER(0, 8192),
			'e', 'f', 'g', 'h',
			[1024 + 2*(sizeof(struct SqshDataMetablock) + 8192)] = METABLOCK_HEADER(0, 8192),
			'i', 'j', 'k', 'l',
			[1024 + 3*(sizeof(struct SqshDataMetablock) + 8192)] = METABLOCK_HEADER(0, 4),
			'm', 'n', 'o', 'p',
			/* clang-format on */
	};
	const uint8_t *p;
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 4);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 4);
	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "abcd", 4) == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8192 * 2, 4);
	assert(rv == 0);
	assert(sqsh__metablock_reader_size(&cursor) == 4);
	p = sqsh__metablock_reader_data(&cursor);
	assert(p != NULL);
	assert(memcmp(p, "ijkl", 4) == 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

static void
advance_overflow(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockReader cursor;
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			[1024] = METABLOCK_HEADER(0, 8192),
			[1024 + sizeof(struct SqshDataMetablock) + 8192 - 1] = 0,
			/* clang-format on */
	};
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_reader_init(&cursor, &sqsh, 1024, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 0, 8192);
	assert(rv == 0);

	rv = sqsh__metablock_reader_advance(&cursor, 8192, 1);
	assert(rv < 0);

	rv = sqsh__metablock_reader_cleanup(&cursor);
	assert(rv == 0);

	sqsh__archive_cleanup(&sqsh);
}

DECLARE_TESTS
TEST(advance_once)
TEST(advance_twice)
TEST(advance_overlapping)
TEST(advance_overlapping_2)
TEST(advance_overlapping_3)
TEST(advance_skip)
TEST(advance_skip_2)
TEST(advance_overflow)
END_TESTS
