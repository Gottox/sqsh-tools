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
#include "../test.h"

#include <sqsh_mapper_private.h>
#include <stdint.h>

static void
init_cursor(void) {
	int rv;
	struct SqshMapManager map_manager = {0};
	struct SqshMapCursor cursor = {0};
	const char buffer[] = "SELECT * FROM table";
	rv = sqsh__map_manager_init(
			&map_manager, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_cursor_init(&cursor, &map_manager, 0, sizeof(buffer) - 1);
	assert(rv == 0);

	sqsh__map_cursor_cleanup(&cursor);
	sqsh__map_manager_cleanup(&map_manager);
}

static void
advance_once(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapCursor cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_cursor_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	assert(rv == 0);

	rv = sqsh__map_cursor_advance(&cursor, 0, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__map_cursor_data(&cursor);
	assert(data == buffer);

	sqsh__map_cursor_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
advance_once_with_offset(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapCursor cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_cursor_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	assert(rv == 0);

	rv = sqsh__map_cursor_advance(&cursor, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__map_cursor_data(&cursor);
	assert(data == &buffer[4]);

	sqsh__map_cursor_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
advance_twice_with_offset(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapCursor cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_cursor_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	assert(rv == 0);

	rv = sqsh__map_cursor_advance(&cursor, 4, 6);
	assert(rv == 0);

	const uint8_t *data = sqsh__map_cursor_data(&cursor);
	assert(data == &buffer[4]);

	rv = sqsh__map_cursor_advance(&cursor, 6, 2);
	assert(rv == 0);

	const uint8_t *data2 = sqsh__map_cursor_data(&cursor);
	assert(data2 == &buffer[10]);

	sqsh__map_cursor_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

static void
advance_to_out_of_bounds(void) {
	int rv;
	struct SqshMapManager mapper = {0};
	struct SqshMapCursor cursor = {0};
	const uint8_t buffer[] = "THIS IS A TEST STRING";
	rv = sqsh__map_manager_init(
			&mapper, buffer,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = sizeof(buffer) - 1});
	assert(rv == 0);

	rv = sqsh__map_cursor_init(&cursor, &mapper, 0, sizeof(buffer) - 1);
	assert(rv == 0);

	rv = sqsh__map_cursor_advance(&cursor, sizeof(buffer) - 1, 1);
	assert(rv != 0);

	sqsh__map_cursor_cleanup(&cursor);
	sqsh__map_manager_cleanup(&mapper);
}

DEFINE
TEST(init_cursor);
TEST(advance_once);
TEST(advance_once_with_offset);
TEST(advance_twice_with_offset);
TEST(advance_to_out_of_bounds);
DEFINE_END
