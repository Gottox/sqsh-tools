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
#include <stdint.h>

int
sqsh__ref_count_array_init(
		struct SqshRefCountArray *array, size_t size, size_t element_size,
		sqsh_ref_count_array_cleanup_t cleanup) {
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
out:
	if (rv < 0) {
		sqsh__ref_count_array_cleanup(array);
	}
	return rv;
}

static void *
get_element(struct SqshRefCountArray *array, int index) {
	sqsh_index_t offset;

	if (SQSH_MULT_OVERFLOW(index, array->element_size, &offset)) {
		return NULL;
	}

	return (void *)&array->data[offset];
}

const void *
sqsh__ref_count_array_set(
		struct SqshRefCountArray *array, int index, void *data, int span) {
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

out:
	pthread_mutex_unlock(&array->mutex);

	return target;
}

const void *
sqsh__ref_count_array_retain(struct SqshRefCountArray *array, int *index) {
	int rv;
	void *data = NULL;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		return NULL;
	}

	if (array->ref_count[*index] < 0) {
		*index = array->ref_count[*index] * -1;
	}

	array->ref_count[*index] += 1;

	data = get_element(array, *index);

	pthread_mutex_unlock(&array->mutex);
	return data;
}

int
sqsh__ref_count_array_release(
		struct SqshRefCountArray *array, const void *element) {
	int rv;

	if (element == NULL) {
		return 0;
	}

	int index = ((uint8_t *)element - array->data) / array->element_size;
	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		// return -SQSH_ERROR_MUTEX_LOCK_FAILED;
		return -SQSH_ERROR_TODO;
	}

	array->ref_count[index] -= 1;

	if (array->ref_count[index] == 0) {
		void *data = get_element(array, index);
		array->cleanup(data);
	}

	pthread_mutex_unlock(&array->mutex);

	return 0;
}

size_t
sqsh__ref_count_array_size(const struct SqshRefCountArray *array) {
	return array->size;
}

int
sqsh__ref_count_array_cleanup(struct SqshRefCountArray *array) {
	void *data;
	for (size_t i = 0; i < array->size; ++i) {
		if (array->ref_count[i] != 0) {
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
