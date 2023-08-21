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
 * @file         cursor.c
 */

#include "../common.h"
#include <testlib.h>

#include "../../include/sqsh_mapper_private.h"
#include <stdint.h>

static void
init_cursor(void) {
	int rv;
	struct SqshMapManager map_manager = {0};
	struct SqshMapIterator cursor = {0};
	const char buffer[] = "SELECT * FROM table";
	rv = sqsh__map_manager_init(
			&map_manager, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_iterator_init(&cursor, &map_manager, 0);
	assert(rv == 0);

	sqsh__map_iterator_cleanup(&cursor);
	sqsh__map_manager_cleanup(&map_manager);
}

static void
next_once(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapIterator cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_iterator_init(&cursor, &mapper, 0);
	assert(rv == 0);

	bool has_next = sqsh__map_iterator_next(&cursor, &rv);
	assert(rv == 0);
	assert(has_next);

	const uint8_t *data = sqsh__map_iterator_data(&cursor);
	assert(data == buffer);

	sqsh__map_iterator_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
next_twice(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapIterator cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 12,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});

	assert(rv == 0);

	rv = sqsh__map_iterator_init(&cursor, &mapper, 0);
	assert(rv == 0);

	bool has_next = sqsh__map_iterator_next(&cursor, &rv);
	assert(rv == 0);

	const uint8_t *data = sqsh__map_iterator_data(&cursor);
	assert(data == &buffer[0]);

	has_next = sqsh__map_iterator_next(&cursor, &rv);
	assert(rv == 0);
	assert(has_next);

	data = sqsh__map_iterator_data(&cursor);
	assert(data == &buffer[12]);

	has_next = sqsh__map_iterator_next(&cursor, &rv);
	assert(rv == 0);
	assert(has_next == false);

	data = sqsh__map_iterator_data(&cursor);
	assert(data == NULL);

	sqsh__map_iterator_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
map_iterator_out_of_bounds_inside_blocksize(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapIterator cursor = {0};
	const uint8_t buffer[] = "12345678901234567890";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 12,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});

	assert(rv == 0);

	rv = sqsh__map_iterator_init(&cursor, &mapper, 22);
	assert(rv != 0);

	sqsh__map_iterator_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
map_iterator_out_of_bounds_outside_blocksize(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapIterator cursor = {0};
	const uint8_t buffer[] = "12345678901234567890";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.mapper_block_size = 12,
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});

	assert(rv == 0);

	rv = sqsh__map_iterator_init(&cursor, &mapper, 30);
	assert(rv != 0);

	sqsh__map_iterator_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

DECLARE_TESTS
TEST(init_cursor)
TEST(next_once)
TEST(next_twice)
TEST(map_iterator_out_of_bounds_inside_blocksize)
TEST(map_iterator_out_of_bounds_outside_blocksize)
END_TESTS
