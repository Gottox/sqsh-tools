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
#include <cextras/types.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

#define LENGTH(x) (sizeof(x) / sizeof(*x))

struct TestCollectIter {
	cx_index_t index;
	char **array;
	size_t size;
	int rv;
};

static int
test_collect_next(void *iter, const char **value, size_t *size) {
	struct TestCollectIter *i = iter;
	if (i->index >= i->size) {
		return i->rv;
	}
	*value = i->array[i->index];
	*size = strlen(i->array[i->index]);
	i->index++;
	return i->rv;
}

static void
collector(void) {
	int rv = 0;
	char *values[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
	char **target = NULL;
	struct TestCollectIter iter = {
			.index = 0,
			.array = values,
			.size = LENGTH(values),
	};

	rv = cx_collect(&target, test_collect_next, &iter);
	assert(rv == 0);

	for (size_t i = 0; i < LENGTH(values); i++) {
		assert(strcmp(values[i], target[i]) == 0);
	}
	assert(target[LENGTH(values)] == NULL);
	free(target);
}

static void
collector_fail(void) {
	int rv = 0;
	char *values[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"};
	char **target = NULL;
	struct TestCollectIter iter = {
			.index = 0,
			.array = values,
			.size = LENGTH(values),
			.rv = -1,
	};

	rv = cx_collect(&target, test_collect_next, &iter);
	assert(rv == -1);
	assert(target == NULL);
}

DECLARE_TESTS
TEST(collector)
TEST(collector_fail)
END_TESTS
