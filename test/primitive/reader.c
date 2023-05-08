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
 * @file         reader.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_primitive_private.h"

struct TestIterator {
	char *data;
	int remaining;
	int block_size;
	int size;
};

static const uint8_t *
test_iter_data(const void *data) {
	struct TestIterator *iter = (struct TestIterator *)data;
	return (uint8_t *)iter->data;
}

static size_t
test_iter_size(const void *data) {
	struct TestIterator *iter = (struct TestIterator *)data;
	return iter->size;
}

static size_t
test_iter_block_size(const void *data) {
	struct TestIterator *iter = (struct TestIterator *)data;
	if (iter->block_size == 0) {
		iter->block_size = strlen(iter->data);
	}
	return iter->block_size;
}

static int
test_iter_skip(void *data, size_t amount, size_t desired_size) {
	struct TestIterator *iter = (struct TestIterator *)data;
	(void)desired_size;
	if (amount > 0) {
		iter->remaining -= amount - 1;
	}
	if (iter->remaining == 0) {
		iter->data = "";
	}
	if (amount > (unsigned int)iter->remaining) {
		return -1;
	}
	iter->remaining--;
	iter->size = strlen(iter->data);
	return iter->size;
}

static int
test_iter_next(void *data, size_t desired_size) {
	return test_iter_skip(data, 1, desired_size);
}

static const struct SqshIteratorImpl impl = {
		.next = test_iter_next,
		.skip = test_iter_skip,
		.data = test_iter_data,
		.size = test_iter_size,
		.block_size = test_iter_block_size};

static void
reader_init(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = ""};

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_advance_once(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 0, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(data == (void *)iter.data);

	sqsh__reader_cleanup(&reader);
}

static void
reader_advance_once_with_offset(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(data == (uint8_t *)&iter.data[4]);

	sqsh__reader_cleanup(&reader);
}

static void
reader_advance_twice_with_offset(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(data == (uint8_t *)&iter.data[4]);

	rv = sqsh__reader_advance(&reader, 6, 2);
	assert(rv == 0);

	const uint8_t *data2 = sqsh__reader_data(&reader);
	assert(data2 == (uint8_t *)&iter.data[10]);

	sqsh__reader_cleanup(&reader);
}

static void
reader_initial_advance(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 5, 3);
	assert(rv == 0);
	const uint8_t *data = sqsh__reader_data(&reader);
	assert(3 == sqsh__reader_size(&reader));
	assert(memcmp(data, "IS ", 3) == 0);
	assert(data == (uint8_t *)&iter.data[5]);

	sqsh__reader_cleanup(&reader);
}

static void
reader_advance_to_out_of_bounds(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, strlen(iter.data), 1);
	assert(rv < 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_advance_over_boundary(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 9, 2);
	assert(rv == 0);
	const uint8_t *data = sqsh__reader_data(&reader);
	assert(2 == sqsh__reader_size(&reader));
	assert(memcmp(data, "90", 2) == 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_initial_advance_2(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = { .data = "ABCD", .remaining = 10 };

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 7, 5);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 5);
	assert(memcmp(data, "DABCD", 5) == 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_error_1(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = { .data = "AB", .remaining = 10 };

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 0, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 4);
	assert(memcmp(data, "ABAB", 4) == 0);

	iter.data = "12";
	rv = sqsh__reader_advance(&reader, 4, 4);
	assert(rv == 0);

	data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 4);
	assert(memcmp(data, "1212", 4) == 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_map_into_buffer(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = { .data = "0123456789", .remaining = 2 };

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 8, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 4);
	assert(memcmp(data, "8901", 4) == 0);

	rv = sqsh__reader_advance(&reader, 1, 4);
	assert(rv == 0);

	data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 4);
	assert(memcmp(data, "9012", 4) == 0);

	sqsh__reader_cleanup(&reader);
}

static void
reader_map_into_buffer_twice(void) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = { .data = "0123456789", .remaining = 3 };

	rv = sqsh__reader_init(&reader, &impl, &iter);
	assert(rv == 0);

	rv = sqsh__reader_advance(&reader, 8, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 4);
	assert(memcmp(data, "8901", 4) == 0);

	rv = sqsh__reader_advance(&reader, 1, 14);
	assert(rv == 0);

	data = sqsh__reader_data(&reader);
	assert(sqsh__reader_size(&reader) == 14);
	assert(memcmp(data, "90123456890123", 14) == 0);

	sqsh__reader_cleanup(&reader);
}

DEFINE
TEST(reader_init);
TEST(reader_advance_once);
TEST(reader_advance_once_with_offset);
TEST(reader_advance_twice_with_offset);
TEST(reader_initial_advance);
TEST(reader_advance_to_out_of_bounds);
TEST(reader_advance_over_boundary);
TEST(reader_initial_advance_2);
TEST(reader_error_1);
TEST(reader_map_into_buffer);
TEST_OFF(reader_map_into_buffer_twice);
DEFINE_END
