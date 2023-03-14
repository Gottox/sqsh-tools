/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         rc_map.c
 */

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"
#include <assert.h>

#if 0
#	include <stdio.h>

static void
debug_print(struct SqshRcMap *array, int index, char msg) {
	fprintf(stderr, "ref_count_array: %i\n", index);
	putc('[', stderr);
	for (sqsh_index_t i = 0; i < array->size; ++i) {
		if (array->ref_count[i] == 0)
			putc('_', stderr);
		else if (array->ref_count[i] >= 10)
			putc('+', stderr);
		else if (array->ref_count[i] < 0)
			putc('<', stderr);
		else
			putc('0' + array->ref_count[i], stderr);
	}
	fputs("]\n", stderr);
	putc(msg, stderr);

	for (sqsh_index_t i = 0; i < (size_t)index; ++i) {
		putc(' ', stderr);
	}
	fputs("^\n", stderr);
}
#else
#	define debug_print(...)
#endif

int
sqsh__rc_map_init(
		struct SqshRcMap *array, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	int rv = 0;
	array->data = calloc(size, element_size);
	if (array->data == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	array->ref_count = calloc(size, sizeof(*array->ref_count));
	if (array->ref_count == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	array->size = size;
	array->cleanup = cleanup;
	array->element_size = element_size;
out:
	if (rv < 0) {
		sqsh__rc_map_cleanup(array);
	}
	return rv;
}

static void *
get_element(struct SqshRcMap *array, sqsh_index_t index) {
	sqsh_index_t offset;

	if (SQSH_MULT_OVERFLOW(index, array->element_size, &offset)) {
		return NULL;
	}

	return (void *)&array->data[offset];
}

int
retain_rc(struct SqshRcMap *array, sqsh_index_t index) {
	int ref_count = ++array->ref_count[index];

	assert(ref_count >= 1);
	debug_print(array, index, '+');

	return ref_count;
}

int
release_rc(struct SqshRcMap *array, sqsh_index_t index) {
	int ref_count = --array->ref_count[index];

	debug_print(array, index, '-');

	assert(ref_count >= 0);

	return ref_count;
}

bool
sqsh__rc_map_is_empty(struct SqshRcMap *array, sqsh_index_t index) {
	return array->ref_count[index] == 0;
}

const void *
sqsh__rc_map_set(
		struct SqshRcMap *array, sqsh_index_t index, void *data, int span) {
	(void)span;
	void *target;

	assert(span == 1);

	target = get_element(array, index);

	if (sqsh__rc_map_is_empty(array, index) == false) {
		array->cleanup(data);
		return NULL;
	}

	memcpy(target, data, array->element_size);

	retain_rc(array, index);

	return target;
}

const void *
sqsh__rc_map_retain(struct SqshRcMap *array, sqsh_index_t *index) {
	void *data = NULL;

	if (sqsh__rc_map_is_empty(array, *index) == false) {
		retain_rc(array, *index);
		debug_print(array, *index, '+');
		data = get_element(array, *index);
	}

	return data;
}

int
sqsh__rc_map_release(struct SqshRcMap *array, const void *element) {
	if (element == NULL) {
		return 0;
	}

	const int index = ((uint8_t *)element - array->data) / array->element_size;

	return sqsh__rc_map_release_index(array, index);
}

int
sqsh__rc_map_release_index(struct SqshRcMap *array, sqsh_index_t index) {
	int ref_count = release_rc(array, index);

	if (ref_count == 0) {
		void *data = get_element(array, index);
		array->cleanup(data);
	}

	return 0;
}

size_t
sqsh__rc_map_size(const struct SqshRcMap *array) {
	return array->size;
}

int
sqsh__rc_map_cleanup(struct SqshRcMap *array) {
	void *data;
	if (array->data != NULL) {
		for (size_t i = 0; i < array->size; ++i) {
			data = get_element(array, i);
			array->cleanup(data);
		}
	}
	free(array->data);
	array->data = NULL;
	free(array->ref_count);
	array->ref_count = NULL;
	array->size = 0;

	return 0;
}

static const void *
lru_rc_map_retain(void *backend, sqsh_index_t index) {
	return sqsh__rc_map_retain(backend, &index);
}

static int
lru_rc_map_release(void *backend, sqsh_index_t index) {
	return sqsh__rc_map_release_index(backend, index);
}

const struct SqshLruBackendImpl sqsh__lru_rc_map = {
		.retain = lru_rc_map_retain,
		.release = lru_rc_map_release,
};
