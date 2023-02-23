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
 * @file         sync_rc_map.c
 */

#include "../common.h"
#include "../test.h"

#include <sqsh_primitive_private.h>

#include <stdatomic.h>
#include <stdint.h>
#include <time.h>

static atomic_uint rc_map_deinit_calls = 0;

static void
rc_map_deinit(void *data) {
	uint8_t *data_ptr = data;
	*data_ptr = UINT8_MAX;
	rc_map_deinit_calls++;
}

static void
init_lru(void) {
	int rv;
	struct SqshLru lru = {0};
	struct SqshSyncRcMap map = {0};

	rv = sqsh__sync_rc_map_init(&map, 128, sizeof(uint8_t), rc_map_deinit);
	assert(rv == 0);

	rv = sqsh__lru_init(&lru, 10, &map);

	rv = sqsh__lru_cleanup(&lru);

	rv = sqsh__sync_rc_map_cleanup(&map);
}

static void *
multithreaded_concurrent_touch_worker(void *arg) {
	struct SqshLru *lru = arg;
	struct SqshSyncRcMap *map = lru->backend;
	size_t size = sqsh__sync_rc_map_size(map);
	int rv;
	const static int repeat_count = 10000;

	for (sqsh_index_t i = 0; i < repeat_count; i++) {
		struct timespec ts = {.tv_sec = 0, .tv_nsec = rand() % 100000};
		int index = rand() % size;
		const uint64_t *get_ptr = sqsh__sync_rc_map_retain(map, &index);
		sqsh__lru_touch(lru, index);
		assert(get_ptr != NULL);
		assert(*get_ptr == (uint64_t)index);
		nanosleep(&ts, NULL);
		rv = sqsh__sync_rc_map_release(map, get_ptr);
		assert(rv == 0);
	}
	return NULL;
}

static void
multithreaded_concurrent_touch(void) {
	int rv;
	const int element_count = 2048;
	struct SqshSyncRcMap map;
	pthread_t threads[16] = {0};
	struct SqshLru lru = {0};

	rv = sqsh__sync_rc_map_init(
			&map, element_count, sizeof(uint64_t), rc_map_deinit);
	assert(rv == 0);
	rv = sqsh__lru_init(&lru, 10, &map);

	for (sqsh_index_t i = 0; i < element_count; i++) {
		int index = i;
		uint64_t data = i;
		const uint64_t *set_ptr = sqsh__sync_rc_map_set(&map, index, &data, 1);
		assert(rv == 0);
		assert(set_ptr != &data);

		assert(rv == 0);
	}

	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_create(
				&threads[i], NULL, multithreaded_concurrent_touch_worker, &lru);
		assert(rv == 0);
	}

	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_join(threads[i], NULL);
		assert(rv == 0);
	}

	rv = sqsh__lru_cleanup(&lru);

	rv = sqsh__sync_rc_map_cleanup(&map);
	assert(rv == 0);
}

DEFINE
TEST(init_lru);
TEST(multithreaded_concurrent_touch);
DEFINE_END
