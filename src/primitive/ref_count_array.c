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

int
sqsh__ref_count_array_init(
		struct SqshRefCountArray *array, size_t size,
		sqsh_ref_count_array_cleanup_t cleanup) {
	int rv = 0;
	array->data = calloc(size, sizeof(void *));
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

int
sqsh__ref_count_array_set(
		struct SqshRefCountArray *array, int index, void *data, int span) {
	int rv;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		// return -SQSH_ERROR_MUTEX_LOCK_FAILED;
		return -SQSH_ERROR_TODO;
	}

	if (array->data != NULL) {
		goto out;
	}

	array->data[index] = data;
	for (int i = 1; i < span; ++i) {
		if (array->data[index + i] == NULL) {
			array->ref_count[index + i] = index * -1;
		}
	}

out:
	pthread_mutex_unlock(&array->mutex);

	return rv;
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

	data = array->data[*index];

	pthread_mutex_unlock(&array->mutex);
	return data;
}

int
sqsh__ref_count_array_release(struct SqshRefCountArray *array, int index) {
	int rv;

	rv = pthread_mutex_lock(&array->mutex);
	if (rv < 0) {
		// return -SQSH_ERROR_MUTEX_LOCK_FAILED;
		return -SQSH_ERROR_TODO;
	}

	array->ref_count[index] -= 1;

	if (array->ref_count[index] == 0) {
		array->cleanup(array->data[index]);
		array->data[index] = NULL;
	}

	pthread_mutex_unlock(&array->mutex);

	return 0;
}

size_t
sqsh__ref_count_array_size(struct SqshRefCountArray *array) {
	return array->size;
}

int
sqsh__ref_count_array_cleanup(struct SqshRefCountArray *array) {
	for (size_t i = 0; i < array->size; ++i) {
		array->cleanup(array->data[i]);
	}
	pthread_mutex_destroy(&array->mutex);
	free(array->data);
	free(array->ref_count);

	return 0;
}
