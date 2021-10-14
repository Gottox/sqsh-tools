/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2018, Enno Boland
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lru_hashmap
 * @created     : Friday Oct 08, 2021 20:24:58 CEST
 */

#include "../common.h"
#include "../test.h"

#include "../../src/utils/lru_hashmap.h"

static char *last_free = NULL;

static int
dtor(void *pointer) {
	last_free = pointer;
	return 0;
}

static int
dummy_dtor(void *pointer) {
	return 0;
}

static void
init_hashmap() {
	int rv = 0;
	struct SquashLruHashmap hashmap = {0};

	rv = squash_lru_hashmap_init(&hashmap, 1024, dummy_dtor);
	assert(rv == 0);

	squash_lru_hashmap_cleanup(&hashmap);
}

static void
add_to_hashmap() {
	int rv = 0;
	struct SquashLruHashmap hashmap = {0};
	char *v1 = "Value1";
	char *v2 = "Value2";

	rv = squash_lru_hashmap_init(&hashmap, 1024, dummy_dtor);
	assert(rv == 0);

	rv = squash_lru_hashmap_put(&hashmap, 1, v1);
	assert(rv == 0);
	rv = squash_lru_hashmap_put(&hashmap, 2, v2);
	assert(rv == 0);

	squash_lru_hashmap_cleanup(&hashmap);
}

static void
read_from_hashmap() {
	int rv = 0;
	struct SquashLruHashmap hashmap = {0};
	char *v1 = "Value1";
	char *v2 = "Value2";
	char *p;

	rv = squash_lru_hashmap_init(&hashmap, 1024, dummy_dtor);
	assert(rv == 0);

	rv = squash_lru_hashmap_put(&hashmap, 1, v1);
	assert(rv == 0);
	rv = squash_lru_hashmap_put(&hashmap, 2, v2);
	assert(rv == 0);

	p = squash_lru_hashmap_get(&hashmap, 1);
	assert(p == v1);
	p = squash_lru_hashmap_get(&hashmap, 2);
	assert(p == v2);

	squash_lru_hashmap_cleanup(&hashmap);
}

static void
hashmap_overflow() {
	int rv = 0;
	struct SquashLruHashmap hashmap = {0};
	char *v1 = "Value1";
	char *v2 = "Value2";
	char *v3 = "Value3";
	char *p;

	rv = squash_lru_hashmap_init(&hashmap, 2, dtor);
	assert(rv == 0);
	assert(hashmap.oldest == NULL);
	assert(hashmap.newest == NULL);

	rv = squash_lru_hashmap_put(&hashmap, 1, v1);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == v1);
	assert(hashmap.newest->pointer == v1);
	assert(last_free == NULL);
	rv = squash_lru_hashmap_put(&hashmap, 2, v2);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == v1);
	assert(hashmap.newest->pointer == v2);
	assert(hashmap.oldest->newer->pointer == v2);
	assert(hashmap.oldest->older == NULL);
	assert(hashmap.newest->older->pointer == v1);
	assert(hashmap.newest->newer == NULL);
	assert(last_free == NULL);
	rv = squash_lru_hashmap_put(&hashmap, 3, v3);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == v2);
	assert(hashmap.newest->pointer == v3);
	assert(hashmap.oldest->newer->pointer == v3);
	assert(hashmap.oldest->older == NULL);
	assert(hashmap.newest->older->pointer == v2);
	assert(hashmap.newest->newer == NULL);
	assert(last_free == v1);

	p = squash_lru_hashmap_get(&hashmap, 1);
	assert(p == NULL);
	p = squash_lru_hashmap_get(&hashmap, 2);
	assert(p == v2);
	p = squash_lru_hashmap_get(&hashmap, 3);
	assert(p == v3);

	rv = squash_lru_hashmap_cleanup(&hashmap);
	assert(rv == 0);
}

static void
hashmap_add_many() {
	const int NBR = 2048, SIZE = 512;
	int rv = 0;
	int length = 0;
	struct SquashLruHashmap hashmap = {0};
	int values[NBR];

	rv = squash_lru_hashmap_init(&hashmap, SIZE, dtor);
	assert(rv == 0);
	assert(hashmap.oldest == NULL);
	assert(hashmap.newest == NULL);

	for (int i = 0; i < NBR; i++) {
		values[i] = i;

		rv = squash_lru_hashmap_put(&hashmap, i, &values[i]);
		assert(rv == 0);
	}

	int *value;

	value = squash_lru_hashmap_get(&hashmap, NBR - SIZE / 2);
	assert(*value == NBR - SIZE / 2);
	value = squash_lru_hashmap_get(&hashmap, NBR - SIZE);
	assert(*value == NBR - SIZE);
	assert(value == hashmap.oldest->pointer);
	value = squash_lru_hashmap_get(&hashmap, NBR - SIZE - 1);
	assert(value == NULL);

	length = 0;
	for (struct SquashLruEntry *entry = hashmap.newest; entry;
			entry = entry->older) {
		assert(NBR - length - 1 == entry->hash);
		length++;
	}
	assert(length == SIZE);

	length = 0;
	for (struct SquashLruEntry *entry = hashmap.oldest; entry;
			entry = entry->newer) {
		assert(NBR - SIZE + length == entry->hash);
		length++;
	}
	assert(length == SIZE);

	rv = squash_lru_hashmap_cleanup(&hashmap);
	assert(rv == 0);
}

static void
hashmap_size_1() {
	int rv = 0;
	struct SquashLruHashmap hashmap = {0};
	char *v1 = "Value1";
	char *v2 = "Value2";
	char *p;

	rv = squash_lru_hashmap_init(&hashmap, 1, dummy_dtor);
	assert(rv == 0);

	rv = squash_lru_hashmap_put(&hashmap, 1, v1);
	assert(rv == 0);
	rv = squash_lru_hashmap_put(&hashmap, 2, v2);
	assert(rv == 0);

	p = squash_lru_hashmap_get(&hashmap, 1);
	assert(p == NULL);
	p = squash_lru_hashmap_get(&hashmap, 2);
	assert(p == v2);

	rv = squash_lru_hashmap_cleanup(&hashmap);
	assert(rv == 0);
}

DEFINE
TEST(init_hashmap);
TEST(add_to_hashmap);
TEST(read_from_hashmap);
TEST(hashmap_overflow);
TEST(hashmap_add_many);
TEST(hashmap_size_1);
DEFINE_END
