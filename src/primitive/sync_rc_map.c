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
 * @file         ref_count_array.c
 */

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

#if 0
#	include <stdio.h>

static void
debug_print(struct SqshSyncRcMap *array, int index, char msg) {
	fprintf(stderr, "ref_count_array: %p %lu\n", (void *)array, array->size);
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
#	define debug_print(a, i, m)
#endif

int
sqsh__sync_rc_map_init(
		struct SqshSyncRcMap *array, size_t size, size_t element_size,
		sqsh_sync_rc_map_cleanup_t cleanup) {
	int rv = 0;
	array->data = calloc(size, element_size);
	if (array->data == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	array->ref_count = calloc(size, sizeof(size_t));
	if (array->ref_count == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	rv = pthread_mutex_init(&array->mutex, NULL);
	array->size = size;
	array->cleanup = cleanup;
	array->element_size = element_size;
out:
	if (rv < 0) {
		sqsh__sync_rc_map_cleanup(array);
	}
	return rv;
}

static void *
get_element(struct SqshSyncRcMap *array, int index) {
	sqsh_index_t offset;

	if (SQSH_MULT_OVERFLOW(index, array->element_size, &offset)) {
		return NULL;
	}

	return (void *)&array->data[offset];
}

const void *
sqsh__sync_rc_map_set(
		struct SqshSyncRcMap *array, int index, void *data, int span) {
	int rv;
	void *target = NULL;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		array->cleanup(data);
		goto out;
	}

	target = get_element(array, index);

	if (array->ref_count[index] != 0) {
		array->cleanup(data);
		goto out;
	}

	memcpy(target, data, array->element_size);
	for (int i = 1; i < span; ++i) {
		if (array->ref_count[index + i] == 0) {
			array->ref_count[index + i] = index * -1;
		}
	}

	array->ref_count[index] = 1;
	debug_print(array, index, 'c');

out:
	pthread_mutex_unlock(&array->mutex);

	return target;
}

const void *
sqsh__sync_rc_map_retain(struct SqshSyncRcMap *array, int *index) {
	int rv;
	void *data = NULL;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		return NULL;
	}

	if (array->ref_count[*index] < 0) {
		*index = array->ref_count[*index] * -1;
	} else if (array->ref_count[*index] != 0) {
		array->ref_count[*index] += 1;
		debug_print(array, *index, '+');
		data = get_element(array, *index);
	}

	pthread_mutex_unlock(&array->mutex);
	return data;
}

int
sqsh__sync_rc_map_release(struct SqshSyncRcMap *array, const void *element) {
	if (element == NULL) {
		return 0;
	}

	int index = ((uint8_t *)element - array->data) / array->element_size;

	return sqsh__sync_rc_map_release_index(array, index);
}

int
sqsh__sync_rc_map_release_index(struct SqshSyncRcMap *array, int index) {
	int rv;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		// return -SQSH_ERROR_MUTEX_LOCK_FAILED;
		return -SQSH_ERROR_TODO;
	}

	array->ref_count[index] -= 1;
	debug_print(array, index, '-');

	if (array->ref_count[index] == 0) {
		void *data = get_element(array, index);
		array->cleanup(data);
	}

	pthread_mutex_unlock(&array->mutex);

	return 0;
}

size_t
sqsh__sync_rc_map_size(const struct SqshSyncRcMap *array) {
	return array->size;
}

int
sqsh__sync_rc_map_cleanup(struct SqshSyncRcMap *array) {
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
	pthread_mutex_destroy(&array->mutex);

	return 0;
}
