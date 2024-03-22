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
 * @file         map_reader.c
 */

#include "../common.h"
#include <utest.h>

#include <sqsh_mapper_private.h>
#include <stdint.h>
#include <string.h>

UTEST(map_reader, init_cursor) {
	int rv;
	struct SqshMapManager map_manager = {0};
	struct SqshMapReader cursor = {0};
	const char buffer[] = "SELECT * FROM table";
	rv = sqsh__map_manager_init(
			&map_manager, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &map_manager, 0, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&map_manager);
}

UTEST(map_reader, advance_once) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 0, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ(buffer, data);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, advance_once_with_offset) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 4, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ(&buffer[4], data);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, advance_twice_with_offset) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 4, 6);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ(&buffer[4], data);

	rv = sqsh__map_reader_advance(&cursor, 6, 2);
	ASSERT_EQ(0, rv);

	const uint8_t *data2 = sqsh__map_reader_data(&cursor);
	ASSERT_EQ(&buffer[10], data2);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, initial_advance) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 4,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 5, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 0, 3);
	ASSERT_EQ(0, rv);
	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ((size_t)3, sqsh__map_reader_size(&cursor));
	ASSERT_EQ(0, memcmp(data, "IS ", 3));
	// ASSERT_EQ(&buffer[5], data);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, advance_to_out_of_bounds) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, sizeof(buffer) - 1, 1);
	ASSERT_NE(0, rv);

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, initial_advance_2) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[1024] = {
			/* clang-format off */
		[96] = 'A', 'B', 'C', 'D', '1', '2', '3', '4',
			/* clang-format on */
	};
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 2,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 96, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 4, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ((size_t)4, sqsh__map_reader_size(&cursor));
	ASSERT_EQ(0, memcmp(data, "1234", 2));

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, error_1) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[1024] = {
			/* clang-format off */
		[96] = 'A', 'B', 'C', 'D', '1', '2', '3', '4',
			/* clang-format on */
	};
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 2,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 96, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_advance(&cursor, 0, 4);
	ASSERT_EQ(0, rv);

	const uint8_t *data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ((size_t)4, sqsh__map_reader_size(&cursor));
	ASSERT_EQ(0, memcmp(data, "ABCD", 4));

	rv = sqsh__map_reader_advance(&cursor, 4, 4);
	ASSERT_EQ(0, rv);

	data = sqsh__map_reader_data(&cursor);
	ASSERT_EQ((size_t)4, sqsh__map_reader_size(&cursor));
	ASSERT_EQ(0, memcmp(data, "1234", 4));

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST(map_reader, error_2) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapReader cursor = {0};
	const uint8_t buffer[1024] = {
			/* clang-format off */
		[96] = 'A', 'B', 'C', 'D', '1', '2', '3', '4',
			/* clang-format on */
	};
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 2,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	ASSERT_EQ(0, rv);

	rv = sqsh__map_reader_init(&cursor, &mapper, 96, sizeof(buffer) - 1);
	ASSERT_EQ(0, rv);

	for (int i = 0; i < 64; i++) {
		rv = sqsh__map_reader_advance(&cursor, 1, 2);
		ASSERT_EQ(0, rv);
	}

	sqsh__map_reader_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

UTEST_MAIN()
