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
#include <stdint.h>
#include <testlib.h>
#include <time.h>

static atomic_uint rc_hash_map_deinit_calls = 0;

static void
rc_hash_map_deinit(void *data) {
	uint8_t *data_ptr = data;
	*data_ptr = UINT8_MAX;
	rc_hash_map_deinit_calls++;
}

static void
init_rc_hash_map(void) {
	int rv;
	struct CxRcHashMap map;

	rv = cx_rc_hash_map_init(&map, 128, sizeof(uint8_t), rc_hash_map_deinit);
	assert(rv == 0);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

static void
set_and_get_element(void) {
	int rv;
	struct CxRcHashMap map;
	uint8_t data = 23;

	rv = cx_rc_hash_map_init(&map, 128, sizeof(uint8_t), rc_hash_map_deinit);
	assert(rv == 0);

	uint64_t key = 4242424;
	const uint8_t *set_ptr = cx_rc_hash_map_put(&map, key, &data);
	assert(rv == 0);
	assert(set_ptr != &data);

	const uint8_t *get_ptr = cx_rc_hash_map_retain(&map, key);
	assert(rv == 0);
	assert(get_ptr != &data);
	assert(*get_ptr == data);

	cx_rc_hash_map_release(&map, set_ptr);
	cx_rc_hash_map_release(&map, get_ptr);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

static void
check_overflow(void) {
	int rv;
	struct CxRcHashMap map;
	uint8_t data = 1;
	const uint8_t *ptr;

	rv = cx_rc_hash_map_init(&map, 4, sizeof(uint8_t), rc_hash_map_deinit);
	assert(rv == 0);

	ptr = cx_rc_hash_map_put(&map, 1, &data);
	assert(*ptr == data);
	ptr = cx_rc_hash_map_put(&map, 2, &data);
	assert(*ptr == data);
	ptr = cx_rc_hash_map_put(&map, 3, &data);
	assert(*ptr == data);
	ptr = cx_rc_hash_map_put(&map, 4, &data);
	assert(*ptr == data);
	data = 42;
	ptr = cx_rc_hash_map_put(&map, 5, &data);
	assert(*ptr == data);

	const uint8_t *get_ptr = cx_rc_hash_map_retain(&map, 5);
	assert(rv == 0);
	assert(*get_ptr == 42);
	cx_rc_hash_map_release(&map, get_ptr);

	cx_rc_hash_map_release_key(&map, 1);
	cx_rc_hash_map_release_key(&map, 2);
	cx_rc_hash_map_release_key(&map, 3);
	cx_rc_hash_map_release_key(&map, 4);
	cx_rc_hash_map_release_key(&map, 5);

	rv = cx_rc_hash_map_cleanup(&map);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(init_rc_hash_map)
TEST(set_and_get_element)
TEST(check_overflow)
END_TESTS
