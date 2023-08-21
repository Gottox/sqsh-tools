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
 * @file         rc_map.c
 */

#include <assert.h>
#include <cextras/collection.h>
#include <stdatomic.h>
#include <testlib.h>

static atomic_uint rc_map_deinit_calls = 0;

static void
deinit(void *data) {
	uint8_t *data_ptr = data;
	*data_ptr = UINT8_MAX;
	rc_map_deinit_calls++;
}

static void
lru_map(void) {
	int rv;
	struct CxLru lru = {0};
	struct CxRcMap map = {0};

	rv = cx_rc_map_init(&map, 128, sizeof(uint8_t), deinit);
	assert(rv == 0);

	rv = cx_lru_init(&lru, 10, &cx_lru_rc_map, &map);
	assert(rv == 0);

	rv = cx_lru_cleanup(&lru);
	assert(rv == 0);

	rv = cx_rc_map_cleanup(&map);
	assert(rv == 0);
}

static void
lru_hash_map(void) {
	int rv;
	struct CxLru lru = {0};
	struct CxRcHashMap map = {0};

	rv = cx_rc_hash_map_init(&map, 128, sizeof(uint8_t), deinit);
	assert(rv == 0);

	rv = cx_lru_init(&lru, 10, &cx_lru_rc_hash_map, &map);
	assert(rv == 0);

	rv = cx_lru_cleanup(&lru);
	assert(rv == 0);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

static void
lru_hash_map_insert_and_retain(void) {
	int rv;
	struct CxLru lru = {0};
	struct CxRcHashMap map = {0};
	uint8_t data = 23;
	const uint8_t *ptr;

	rv = cx_rc_hash_map_init(&map, 128, sizeof(uint8_t), deinit);
	assert(rv == 0);

	rv = cx_lru_init(&lru, 10, &cx_lru_rc_hash_map, &map);
	assert(rv == 0);

	ptr = cx_rc_hash_map_put(&map, 42, &data);
	rv = cx_lru_touch(&lru, 42);
	assert(rv == 0);

	cx_rc_hash_map_release(&map, ptr);

	rv = cx_lru_cleanup(&lru);
	assert(rv == 0);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

static void
lru_hash_map_insert_and_retain_twice(void) {
	int rv;
	struct CxLru lru = {0};
	struct CxRcHashMap map = {0};
	uint8_t data = 23;
	const uint8_t *ptr;

	rv = cx_rc_hash_map_init(&map, 128, sizeof(uint8_t), deinit);
	assert(rv == 0);

	rv = cx_lru_init(&lru, 10, &cx_lru_rc_hash_map, &map);
	assert(rv == 0);

	ptr = cx_rc_hash_map_put(&map, 42, &data);
	rv = cx_lru_touch(&lru, 42);
	assert(rv == 0);
	cx_rc_hash_map_release(&map, ptr);

	ptr = cx_rc_hash_map_put(&map, 36, &data);
	rv = cx_lru_touch(&lru, 36);
	assert(rv == 0);

	rv = cx_lru_touch(&lru, 42);
	assert(rv == 0);

	cx_rc_hash_map_release(&map, ptr);

	rv = cx_lru_cleanup(&lru);
	assert(rv == 0);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

static void
lru_hash_map_insert_and_retain_overflow(void) {
	int rv;
	struct CxLru lru = {0};
	struct CxRcHashMap map = {0};
	uint8_t data = 232;
	const uint8_t *ptr;

	rv = cx_rc_hash_map_init(&map, 10, sizeof(uint8_t), deinit);
	assert(rv == 0);

	rv = cx_lru_init(&lru, 10, &cx_lru_rc_hash_map, &map);
	assert(rv == 0);

	ptr = cx_rc_hash_map_put(&map, 0, &data);
	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	cx_rc_hash_map_release(&map, ptr);

	ptr = cx_rc_hash_map_put(&map, 1, &data);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);
	cx_rc_hash_map_release(&map, ptr);

	ptr = cx_rc_hash_map_put(&map, 2, &data);
	rv = cx_lru_touch(&lru, 2);
	assert(rv == 0);
	cx_rc_hash_map_release(&map, ptr);

	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);

	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);

	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);

	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);

	rv = cx_lru_touch(&lru, 0);
	assert(rv == 0);
	rv = cx_lru_touch(&lru, 1);
	assert(rv == 0);

	ptr = cx_rc_hash_map_retain(&map, 2);
	assert(ptr == NULL);

	rv = cx_lru_cleanup(&lru);
	assert(rv == 0);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(lru_map)
TEST(lru_hash_map)
TEST(lru_hash_map_insert_and_retain)
TEST(lru_hash_map_insert_and_retain_twice)
TEST(lru_hash_map_insert_and_retain_overflow)
END_TESTS
