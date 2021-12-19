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

static struct HsqsRefCount *last_free = NULL;

static int
dtor(void *pointer) {
	assert(pointer != NULL);
	last_free = &((struct HsqsRefCount *)pointer)[-1];
	return 0;
}

static int
dummy_dtor(void *pointer) {
	assert(pointer != NULL);
	return 0;
}

static void
init_hashmap() {
	int rv = 0;
	struct HsqsLruHashmap hashmap = {0};

	rv = hsqs_lru_hashmap_init(&hashmap, 1024);
	assert(rv == 0);

	hsqs_lru_hashmap_cleanup(&hashmap);
}

static void
add_to_hashmap() {
	int rv = 0;
	struct HsqsLruHashmap hashmap = {0};
	struct HsqsRefCount *rc1;
	struct HsqsRefCount *rc2;

	rv = hsqs_ref_count_new(&rc1, sizeof(int), dummy_dtor);
	assert(rv == 0);
	rv = hsqs_ref_count_new(&rc2, sizeof(int), dummy_dtor);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_init(&hashmap, 1024);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_put(&hashmap, 1, rc1);
	assert(rv == 0);
	rv = hsqs_lru_hashmap_put(&hashmap, 2, rc2);
	assert(rv == 0);

	hsqs_lru_hashmap_cleanup(&hashmap);
}

static void
read_from_hashmap() {
	int rv = 0;
	struct HsqsLruHashmap hashmap = {0};
	struct HsqsRefCount *rc1;
	struct HsqsRefCount *rc2;
	struct HsqsRefCount *p;

	rv = hsqs_ref_count_new(&rc1, sizeof(int), dummy_dtor);
	assert(rv == 0);
	rv = hsqs_ref_count_new(&rc2, sizeof(int), dummy_dtor);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_init(&hashmap, 1024);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_put(&hashmap, 1, rc1);
	assert(rv == 0);
	rv = hsqs_lru_hashmap_put(&hashmap, 2, rc2);
	assert(rv == 0);

	p = hsqs_lru_hashmap_get(&hashmap, 1);
	assert(p == rc1);
	p = hsqs_lru_hashmap_get(&hashmap, 2);
	assert(p == rc2);

	assert(hashmap.oldest->pointer == rc1);
	assert(hashmap.newest->pointer == rc2);

	hsqs_lru_hashmap_cleanup(&hashmap);
}

static void
hashmap_overflow() {
	int rv = 0;
	struct HsqsLruHashmap hashmap = {0};
	struct HsqsRefCount *rc1;
	struct HsqsRefCount *rc2;
	struct HsqsRefCount *rc3;
	struct HsqsRefCount *p;

	rv = hsqs_ref_count_new(&rc1, sizeof(int), dtor);
	assert(rv == 0);
	rv = hsqs_ref_count_new(&rc2, sizeof(int), dtor);
	assert(rv == 0);
	rv = hsqs_ref_count_new(&rc3, sizeof(int), dtor);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_init(&hashmap, 2);
	assert(rv == 0);
	assert(hashmap.oldest == NULL);
	assert(hashmap.newest == NULL);

	rv = hsqs_lru_hashmap_put(&hashmap, 1, rc1);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == rc1);
	assert(hashmap.newest->pointer == rc1);
	assert(last_free == NULL);
	rv = hsqs_lru_hashmap_put(&hashmap, 2, rc2);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == rc1);
	assert(hashmap.newest->pointer == rc2);
	assert(hashmap.oldest->newer->pointer == rc2);
	assert(hashmap.oldest->older == NULL);
	assert(hashmap.newest->older->pointer == rc1);
	assert(hashmap.newest->newer == NULL);
	assert(last_free == NULL);
	rv = hsqs_lru_hashmap_put(&hashmap, 3, rc3);
	assert(rv == 0);
	assert(hashmap.oldest->pointer == rc2);
	assert(hashmap.newest->pointer == rc3);
	assert(hashmap.oldest->newer->pointer == rc3);
	assert(hashmap.oldest->older == NULL);
	assert(hashmap.newest->older->pointer == rc2);
	assert(hashmap.newest->newer == NULL);
	assert(last_free == rc1);

	p = hsqs_lru_hashmap_get(&hashmap, 1);
	assert(p == NULL);
	p = hsqs_lru_hashmap_get(&hashmap, 2);
	assert(p == rc2);
	p = hsqs_lru_hashmap_get(&hashmap, 3);
	assert(p == rc3);

	rv = hsqs_lru_hashmap_cleanup(&hashmap);
	assert(rv == 0);
}

static void
hashmap_add_many() {
	const int NBR = 2048, SIZE = 512;
	int rv = 0;
	int length = 0;
	struct HsqsLruHashmap hashmap = {0};
	struct HsqsRefCount *values[NBR];
	struct HsqsRefCount *rc;

	int *value;

	rv = hsqs_lru_hashmap_init(&hashmap, SIZE);
	assert(rv == 0);
	assert(hashmap.oldest == NULL);
	assert(hashmap.newest == NULL);

	for (int i = 0; i < NBR; i++) {
		rv = hsqs_ref_count_new(&values[i], sizeof(int), dummy_dtor);
		assert(rv == 0);
		value = hsqs_ref_count_retain(values[i]);
		*value = i;

		rv = hsqs_lru_hashmap_put(&hashmap, i, values[i]);
		assert(rv == 0);
		hsqs_ref_count_release(values[i]);
	}

	length = 0;
	for (struct HsqsLruEntry *entry = hashmap.newest; entry;
		 entry = entry->older) {
		assert(NBR - length - 1 == (int)entry->hash);
		length++;
	}
	assert(length == SIZE);

	length = 0;
	for (struct HsqsLruEntry *entry = hashmap.oldest; entry;
		 entry = entry->newer) {
		assert(NBR - SIZE + length == (int)entry->hash);
		length++;
	}
	assert(length == SIZE);

	rc = hsqs_lru_hashmap_get(&hashmap, NBR - SIZE / 2);
	value = hsqs_ref_count_retain(rc);
	assert(*value == NBR - SIZE / 2);
	hsqs_ref_count_release(rc);

	rc = hsqs_lru_hashmap_get(&hashmap, NBR - SIZE);
	value = hsqs_ref_count_retain(rc);
	assert(*value == NBR - SIZE);
	hsqs_ref_count_release(rc);

	rc = hsqs_lru_hashmap_get(&hashmap, NBR - SIZE - 1);
	assert(rc == NULL);

	length = 0;
	for (struct HsqsLruEntry *entry = hashmap.oldest; entry;
		 entry = entry->newer) {
		length++;
	}
	assert(length == SIZE);

	rv = hsqs_lru_hashmap_cleanup(&hashmap);
	assert(rv == 0);
}

static void
hashmap_size_1() {
	int rv = 0;
	struct HsqsLruHashmap hashmap = {0};
	struct HsqsRefCount *rc1;
	struct HsqsRefCount *rc2;
	struct HsqsRefCount *p;

	rv = hsqs_ref_count_new(&rc1, sizeof(int), dummy_dtor);
	assert(rv == 0);
	rv = hsqs_ref_count_new(&rc2, sizeof(int), dummy_dtor);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_init(&hashmap, 1);
	assert(rv == 0);

	rv = hsqs_lru_hashmap_put(&hashmap, 1, rc1);
	assert(rv == 0);
	rv = hsqs_lru_hashmap_put(&hashmap, 2, rc2);
	assert(rv == 0);

	p = hsqs_lru_hashmap_get(&hashmap, 1);
	assert(p == NULL);
	p = hsqs_lru_hashmap_get(&hashmap, 2);
	assert(p == rc2);

	rv = hsqs_lru_hashmap_cleanup(&hashmap);
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
