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
 * @file         metablock_iterator.c
 */

#include "../common.h"
#include <testlib.h>

#include <mksqsh_metablock.h>
#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_metablock_private.h>
#include <stdint.h>

static void
map_iterator__next_once(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {0};
	const uint8_t *p;
	FILE *farchive = test_sqsh_prepare_archive(payload, sizeof(payload));

	struct MksqshMetablock metablock_writer = {0};
	mksqsh__metablock_init(&metablock_writer, farchive);
	mksqsh__metablock_write(&metablock_writer, (const uint8_t *)"abcd", 4);
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_cleanup(&metablock_writer);

	test_sqsh_init_archive(&sqsh, farchive, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(0, rv);
	assert(has_next);

	ASSERT_EQ((size_t)4, sqsh__metablock_iterator_size(&iter));

	p = sqsh__metablock_iterator_data(&iter);
	ASSERT_NE(NULL, p);
	ASSERT_EQ(0, memcmp(p, "abcd", 4));

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

static void
map_iterator__next_failing_with_overflow(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {SQSH_HEADER, METABLOCK_HEADER(0, 8192)};
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);
	assert(!has_next);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

static void
map_iterator__next_failing_with_size_too_big(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192 * 2] = {SQSH_HEADER, METABLOCK_HEADER(0, 8192 + 1)};
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(-SQSH_ERROR_SIZE_MISMATCH, rv);
	assert(!has_next);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

static void
map_iterator__next_failing_with_no_compression(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {
			SQSH_HEADER, METABLOCK_HEADER(1, 4), 'a', 'b', 'c', 'd',
	};
	mk_stub(&sqsh, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_NE(0, rv);
	assert(!has_next);

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

static void
map_iterator__next_twice(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshMetablockIterator iter;
	uint8_t payload[8192] = {0};
	const uint8_t *p;
	FILE *farchive = test_sqsh_prepare_archive(payload, sizeof(payload));

	struct MksqshMetablock metablock_writer = {0};
	mksqsh__metablock_init(&metablock_writer, farchive);
	mksqsh__metablock_write(&metablock_writer, (const uint8_t *)"abcd", 4);
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_write(&metablock_writer, (const uint8_t *)"efgh", 4);
	mksqsh__metablock_flush(&metablock_writer);
	mksqsh__metablock_cleanup(&metablock_writer);

	test_sqsh_init_archive(&sqsh, farchive, payload, sizeof(payload));

	rv = sqsh__metablock_iterator_init(
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(0, rv);
	assert(has_next);

	ASSERT_EQ((size_t)4, sqsh__metablock_iterator_size(&iter));

	p = sqsh__metablock_iterator_data(&iter);
	ASSERT_NE(NULL, p);
	ASSERT_EQ(0, memcmp(p, "abcd", 4));

	has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(0, rv);
	assert(has_next);

	ASSERT_EQ((size_t)4, sqsh__metablock_iterator_size(&iter));

	p = sqsh__metablock_iterator_data(&iter);
	ASSERT_NE(NULL, p);
	ASSERT_EQ(0, memcmp(p, "efgh", 4));

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

static void
map_iterator__next_compressed(void) {
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
			&iter, &sqsh, sizeof(struct SqshDataSuperblock), sizeof(payload));
	ASSERT_EQ(0, rv);

	bool has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(0, rv);
	assert(has_next);

	ASSERT_EQ((size_t)4, sqsh__metablock_iterator_size(&iter));

	p = sqsh__metablock_iterator_data(&iter);
	ASSERT_NE(NULL, p);
	ASSERT_EQ(0, memcmp(p, "abcd", 4));

	has_next = sqsh__metablock_iterator_next(&iter, &rv);
	ASSERT_EQ(0, rv);
	assert(has_next);

	ASSERT_EQ((size_t)4, sqsh__metablock_iterator_size(&iter));

	p = sqsh__metablock_iterator_data(&iter);
	ASSERT_NE(NULL, p);
	ASSERT_EQ(0, memcmp(p, "efgh", 4));

	rv = sqsh__metablock_iterator_cleanup(&iter);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&sqsh);
}

DECLARE_TESTS
TEST(map_iterator__next_once)
TEST(map_iterator__next_failing_with_overflow)
TEST(map_iterator__next_failing_with_size_too_big)
TEST(map_iterator__next_failing_with_no_compression)
TEST(map_iterator__next_twice)
TEST(map_iterator__next_compressed)
END_TESTS
