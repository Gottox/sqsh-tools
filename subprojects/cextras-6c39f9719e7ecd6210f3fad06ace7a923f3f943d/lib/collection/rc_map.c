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

#include "../../include/cextras/collection.h"
#include "../../include/cextras/error.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if 0
#	include <stdio.h>

static void
debug_print(struct CxRcMap *array, int index, char msg) {
	fprintf(stderr, "ref_count_array: %i\n", index);
	putc('[', stderr);
	for (size_t i = 0; i < array->size; ++i) {
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

	for (size_t i = 0; i < (size_t)index; ++i) {
		putc(' ', stderr);
	}
	fputs("^\n", stderr);
}
#else
#	define debug_print(...)
#endif

int
cx_rc_map_init(
		struct CxRcMap *array, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	int rv = 0;
	array->data = calloc(size, element_size);
	if (array->data == NULL) {
		rv = -CX_ERR_ALLOC;
		goto out;
	}
	array->ref_count = calloc(size, sizeof(*array->ref_count));
	if (array->ref_count == NULL) {
		rv = -CX_ERR_ALLOC;
		goto out;
	}

	array->size = size;
	array->cleanup = cleanup;
	array->element_size = element_size;
out:
	if (rv < 0) {
		cx_rc_map_cleanup(array);
	}
	return rv;
}

static void *
get_element(struct CxRcMap *array, size_t index) {
	size_t offset;

	if (CX_MUL_OVERFLOW(index, array->element_size, &offset)) {
		return NULL;
	}

	return (void *)&array->data[offset];
}

int
retain_rc(struct CxRcMap *array, size_t index) {
	int ref_count = ++array->ref_count[index];

	assert(ref_count >= 1);
	debug_print(array, index, '+');

	return ref_count;
}

int
release_rc(struct CxRcMap *array, size_t index) {
	int ref_count = --array->ref_count[index];

	debug_print(array, index, '-');

	assert(ref_count >= 0);

	return ref_count;
}

bool
cx_rc_map_is_empty(struct CxRcMap *array, size_t index) {
	return array->ref_count[index] == 0;
}

const void *
cx_rc_map_set(struct CxRcMap *array, size_t index, void *data) {
	void *target;

	target = get_element(array, index);

	/* If the element is already in the array, cleanup the new data and retain
	 * the old. */
	if (cx_rc_map_is_empty(array, index) == false) {
		array->cleanup(data);
		return cx_rc_map_retain(array, index);
	}

	memcpy(target, data, array->element_size);

	retain_rc(array, index);

	return target;
}

const void *
cx_rc_map_retain(struct CxRcMap *array, size_t index) {
	void *data = NULL;

	if (cx_rc_map_is_empty(array, index) == false) {
		retain_rc(array, index);
		debug_print(array, index, '+');
		data = get_element(array, index);
	}

	return data;
}

int
cx_rc_map_release(struct CxRcMap *array, const void *element) {
	if (element == NULL) {
		return 0;
	}

	const size_t index =
			((uint8_t *)element - array->data) / array->element_size;

	return cx_rc_map_release_index(array, index);
}

int
cx_rc_map_release_index(struct CxRcMap *array, size_t index) {
	int ref_count = release_rc(array, index);

	if (ref_count == 0) {
		void *data = get_element(array, index);
		array->cleanup(data);
	}

	return 0;
}

bool
cx_rc_map_contains(struct CxRcMap *array, const void *element) {
	const uint8_t *needle = (uint8_t *)element;

	if (needle == NULL) {
		return false;
	}

	if (needle < array->data) {
		return false;
	}

	if (needle >= &array->data[array->size * array->element_size]) {
		return false;
	}

	return true;
}

size_t
cx_rc_map_size(const struct CxRcMap *array) {
	return array->size;
}

int
cx_rc_map_cleanup(struct CxRcMap *array) {
#ifndef NDEBUG
	if (array->ref_count != NULL) {
		int acc = 0;
		for (size_t i = 0; i < array->size; ++i) {
			acc |= array->ref_count[i];
		}
		assert(acc == 0);
	}
#endif
	free(array->data);
	array->data = NULL;
	free(array->ref_count);
	array->ref_count = NULL;
	array->size = 0;

	return 0;
}

static const void *
lru_rc_map_retain(void *backend, size_t index) {
	return cx_rc_map_retain(backend, index);
}

static int
lru_rc_map_release(void *backend, size_t index) {
	return cx_rc_map_release_index(backend, index);
}

const struct CxLruBackendImpl cx_lru_rc_map = {
		.retain = lru_rc_map_retain,
		.release = lru_rc_map_release,
};
