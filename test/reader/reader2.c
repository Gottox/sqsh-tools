/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         integration.c
 */

#include "../include/sqsh_reader_private.h"
#include "common.h"
#include <sys/wait.h>
#include <testlib.h>

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

static int
test_iter_next(void *data, size_t desired_size) {
	struct TestIterator *iter = (struct TestIterator *)data;
	(void)desired_size;
	if (iter->remaining == 0) {
		iter->data = "";
		return 0;
	}
	iter->remaining--;
	iter->size = strlen(iter->data);
	return iter->size;
}

static const struct SqshReader2IteratorImpl test_iter = {
		.next = test_iter_next,
		.data = test_iter_data,
		.size = test_iter_size,
};

static void
test_reader2_init(void) {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_with_offset(void) {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 1,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 1, 2);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 2);
	assert(memcmp(data, "es", 2) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_to_block(void) {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 1,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 0, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 4);
	assert(memcmp(data, "test", 4) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_to_two_blocks() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 0, 8);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 8);
	assert(memcmp(data, "testtest", 8) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_to_two_blocks_with_offset() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 1, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "esttes", 6) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_from_buffered_to_mapped() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	// Trigger a buffered result
	rv = sqsh__reader2_advance(&reader, 1, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "esttes", 6) == 0);

	// Trigger back to a mapped result
	rv = sqsh__reader2_advance(&reader, 4, 3);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	size = sqsh__reader2_size(&reader);
	assert(size == 3);
	assert(memcmp(data, "est", 3) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_from_buffered_to_buffered() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	// Trigger a buffered result
	rv = sqsh__reader2_advance(&reader, 1, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "esttes", 6) == 0);

	// Trigger back to a mapped result
	rv = sqsh__reader2_advance(&reader, 8, 6);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "esttes", 6) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_inside_buffered() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	// Trigger a buffered result
	rv = sqsh__reader2_advance(&reader, 1, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "esttes", 6) == 0);

	// Trigger back to a mapped result
	rv = sqsh__reader2_advance(&reader, 1, 6);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	size = sqsh__reader2_size(&reader);
	assert(size == 6);
	assert(memcmp(data, "sttest", 6) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_with_zero_size() {
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 3, 0);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	size_t size = sqsh__reader2_size(&reader);
	assert(size == 0);

	// Trigger back to a mapped result
	rv = sqsh__reader2_advance(&reader, 0, 4);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	size = sqsh__reader2_size(&reader);
	assert(size == 4);
	assert(memcmp(data, "ttes", 4) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_once(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 0, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(data == (void *)iter.data);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_once_with_offset(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(data == (uint8_t *)&iter.data[4]);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_twice_with_offset(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(data == (uint8_t *)&iter.data[4]);

	rv = sqsh__reader2_advance(&reader, 6, 2);
	assert(rv == 0);

	const uint8_t *data2 = sqsh__reader2_data(&reader);
	assert(data2 == (uint8_t *)&iter.data[10]);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_initial_advance(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 5, 3);
	assert(rv == 0);
	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(3 == sqsh__reader2_size(&reader));
	assert(memcmp(data, "IS ", 3) == 0);
	assert(data == (uint8_t *)&iter.data[5]);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_to_out_of_bounds(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, strlen(iter.data), 1);
	assert(rv < 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_advance_over_boundary(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 9, 2);
	assert(rv == 0);
	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(2 == sqsh__reader2_size(&reader));
	assert(memcmp(data, "90", 2) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_initial_advance_2(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {.data = "ABCD", .remaining = 10};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 7, 5);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 5);
	assert(memcmp(data, "DABCD", 5) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_error_1(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {.data = "AB", .remaining = 10};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 0, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 4);
	assert(memcmp(data, "ABAB", 4) == 0);

	iter.data = "12";
	rv = sqsh__reader2_advance(&reader, 4, 4);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 4);
	assert(memcmp(data, "1212", 4) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_map_into_buffer(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 8, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 4);
	assert(memcmp(data, "8901", 4) == 0);

	rv = sqsh__reader2_advance(&reader, 1, 4);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 4);
	assert(memcmp(data, "9012", 4) == 0);

	sqsh__reader2_cleanup(&reader);
}

static void
test_reader2_map_into_buffer_twice(void) {
	int rv;
	struct SqshReader2 reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 3};

	rv = sqsh__reader2_init(&reader, &test_iter, &iter);
	assert(rv == 0);

	rv = sqsh__reader2_advance(&reader, 8, 4);
	assert(rv == 0);

	const uint8_t *data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 4);
	assert(memcmp(data, "8901", 4) == 0);

	rv = sqsh__reader2_advance(&reader, 1, 14);
	assert(rv == 0);

	data = sqsh__reader2_data(&reader);
	assert(sqsh__reader2_size(&reader) == 14);
	assert(memcmp(data, "90123456789012", 14) == 0);

	sqsh__reader2_cleanup(&reader);
}

DECLARE_TESTS
TEST(test_reader2_init)
TEST(test_reader2_advance_to_block)
TEST(test_reader2_advance_with_offset)
TEST(test_reader2_advance_to_two_blocks)
TEST(test_reader2_advance_to_two_blocks_with_offset)
TEST(test_reader2_from_buffered_to_mapped)
TEST(test_reader2_from_buffered_to_buffered)
TEST(test_reader2_from_buffered_to_buffered)
TEST(test_reader2_advance_inside_buffered)
TEST(test_reader2_advance_with_zero_size)
TEST(test_reader2_advance_once)
TEST(test_reader2_advance_once_with_offset)
TEST(test_reader2_advance_twice_with_offset)
TEST(test_reader2_initial_advance)
TEST(test_reader2_advance_to_out_of_bounds)
TEST(test_reader2_advance_over_boundary)
TEST(test_reader2_initial_advance_2)
TEST(test_reader2_error_1)
TEST(test_reader2_map_into_buffer)
TEST(test_reader2_map_into_buffer_twice)
END_TESTS
