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

static atomic_uint rc_map_deinit_calls = 0;

static void
rc_map_deinit(void *data) {
	uint8_t *data_ptr = data;
	*data_ptr = UINT8_MAX;
	rc_map_deinit_calls++;
}

static void
init_rc_map(void) {
	int rv;
	struct CxRcMap map;

	rv = cx_rc_map_init(&map, 128, sizeof(uint8_t), rc_map_deinit);
	assert(rv == 0);

	rv = cx_rc_map_cleanup(&map);
	assert(rv == 0);
}

static void
set_and_get_element(void) {
	int rv;
	struct CxRcMap map;
	uint8_t data = 23;

	rv = cx_rc_map_init(&map, 128, sizeof(uint8_t), rc_map_deinit);
	assert(rv == 0);

	size_t index = 42;
	const uint8_t *set_ptr = cx_rc_map_set(&map, index, &data);
	assert(rv == 0);
	assert(set_ptr != &data);

	const uint8_t *get_ptr = cx_rc_map_retain(&map, index);
	assert(rv == 0);
	assert(get_ptr != &data);
	assert(get_ptr == get_ptr);
	cx_rc_map_release(&map, get_ptr);

	cx_rc_map_release(&map, set_ptr);

	rv = cx_rc_map_cleanup(&map);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(init_rc_map)
TEST(set_and_get_element)
END_TESTS
