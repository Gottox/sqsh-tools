/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         reader.c
 */

#include "../common.h"
#include <sqsh_error.h>
#include <sqsh_reader_private.h>
#include <sys/wait.h>
#include <utest.h>

struct TestIterator {
	char *data;
	char *current_data;
	int remaining;
};

static const uint8_t *
test_iter_data(const void *data) {
	struct TestIterator *iter = (struct TestIterator *)data;
	return (uint8_t *)iter->current_data;
}

static size_t
test_iter_size(const void *data) {
	struct TestIterator *iter = (struct TestIterator *)data;
	if (iter->current_data == NULL) {
		return 0;
	} else {
		return strlen(iter->current_data);
	}
}

static bool
test_iter_next(void *data, size_t desired_size, int *err) {
	struct TestIterator *iter = (struct TestIterator *)data;
	(void)desired_size;
	(void)err;
	if (iter->remaining == 0) {
		iter->current_data = "";
		return false;
	}
	iter->current_data = iter->data;
	iter->remaining--;

	return true;
}

static int
test_iter_skip(void *iterator, uint64_t *offset, size_t desired_size) {
	int rv = 0;

	size_t current_size = test_iter_size(iterator);
	while (current_size <= *offset) {
		*offset -= current_size;
		bool has_next = test_iter_next(iterator, desired_size, &rv);
		if (rv < 0) {
			goto out;
		} else if (!has_next) {
			rv = -SQSH_ERROR_OUT_OF_BOUNDS;
			goto out;
		}
		current_size = test_iter_size(iterator);
	}

	rv = 0;
out:
	return rv;
}

static const struct SqshReaderIteratorImpl test_iter = {
		.next = test_iter_next,
		.skip = test_iter_skip,
		.data = test_iter_data,
		.size = test_iter_size,
};

UTEST(reader, test_reader_init) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_with_offset) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 1,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 1, 2);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)2, size);
	ASSERT_EQ(0, memcmp(data, "es", 2));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_to_block) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 1,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 0, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(data, "test", 4));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_to_two_blocks) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 0, 8);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)8, size);
	ASSERT_EQ(0, memcmp(data, "testtest", 8));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_to_two_blocks_with_offset) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 1, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "esttes", 6));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_from_buffered_to_mapped) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	// Trigger a buffered result
	rv = sqsh__reader_advance(&reader, 1, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "esttes", 6));

	// Trigger back to a mapped result
	rv = sqsh__reader_advance(&reader, 4, 3);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)3, size);
	ASSERT_EQ(0, memcmp(data, "est", 3));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_from_buffered_to_buffered) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	// Trigger a buffered result
	rv = sqsh__reader_advance(&reader, 1, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "esttes", 6));

	// Trigger back to a mapped result
	rv = sqsh__reader_advance(&reader, 8, 6);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "esttes", 6));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_inside_buffered) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	// Trigger a buffered result
	rv = sqsh__reader_advance(&reader, 1, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "esttes", 6));

	// Trigger back to a mapped result
	rv = sqsh__reader_advance(&reader, 1, 6);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)6, size);
	ASSERT_EQ(0, memcmp(data, "sttest", 6));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_with_zero_size) {
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "test",
			.remaining = 4,
	};

	int rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 3, 0);
	ASSERT_EQ(0, rv);

	size_t size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)0, size);

	// Trigger back to a mapped result
	rv = sqsh__reader_advance(&reader, 0, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	size = sqsh__reader_size(&reader);
	ASSERT_EQ((size_t)4, size);
	ASSERT_EQ(0, memcmp(data, "ttes", 4));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_once) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 0, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((void *)iter.data, data);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_once_with_offset) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 4, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((uint8_t *)&iter.data[4], data);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_twice_with_offset) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 4, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((uint8_t *)&iter.data[4], data);

	rv = sqsh__reader_advance(&reader, 6, 2);
	ASSERT_EQ(0, rv);

	const uint8_t *data2 = sqsh__reader_data(&reader);
	ASSERT_EQ((uint8_t *)&iter.data[10], data2);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_initial_advance) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};
	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 5, 3);
	ASSERT_EQ(0, rv);
	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)3, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "IS ", 3));
	ASSERT_EQ((uint8_t *)&iter.data[5], data);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_to_out_of_bounds) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {
			.data = "THIS IS A TEST STRING", .remaining = 1};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, strlen(iter.data), 1);
	assert(rv < 0);

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_advance_over_boundary) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 9, 2);
	ASSERT_EQ(0, rv);
	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)2, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "90", 2));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_initial_advance_2) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "ABCD", .remaining = 10};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 7, 5);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)5, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "DABCD", 5));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_error_1) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "AB", .remaining = 10};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 0, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "ABAB", 4));

	iter.data = "12";
	rv = sqsh__reader_advance(&reader, 4, 4);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "1212", 4));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_map_into_buffer) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 8, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "8901", 4));

	rv = sqsh__reader_advance(&reader, 1, 4);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "9012", 4));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_map_into_buffer_twice) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 3};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 8, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "8901", 4));

	rv = sqsh__reader_advance(&reader, 1, 14);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)14, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "90123456789012", 14));

	sqsh__reader_cleanup(&reader);
}

UTEST(reader, test_reader_extend_size_till_end) {
	int rv;
	struct SqshReader reader = {0};
	struct TestIterator iter = {.data = "0123456789", .remaining = 2};

	rv = sqsh__reader_init(&reader, &test_iter, &iter);
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 4, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)4, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "4567", 4));

	rv = sqsh__reader_advance(&reader, 0, 6);
	ASSERT_EQ(0, rv);

	data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)6, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "456789", 6));

	iter.data = "ABCDEF";

	rv = sqsh__reader_advance(&reader, 0, 12);
	data = sqsh__reader_data(&reader);
	ASSERT_EQ((size_t)12, sqsh__reader_size(&reader));
	ASSERT_EQ(0, memcmp(data, "456789ABCDEF", 12));
	ASSERT_EQ(0, rv);

	rv = sqsh__reader_advance(&reader, 0, 13);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);

	sqsh__reader_cleanup(&reader);
}

UTEST_MAIN()
