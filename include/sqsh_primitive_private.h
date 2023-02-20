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
 * @file         sqsh_primitive_private.h
 */

#ifndef SQSH_PRIMITIVE_PRIVATE_H
#define SQSH_PRIMITIVE_PRIVATE_H

#include "sqsh_common.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// primitive/buffer.c

/**
 * @brief The SqshBuffer struct is a buffer for arbitrary data.
 *
 * The buffer takes care about resizing and freeing the memory managed by The
 * buffer.
 */
struct SqshBuffer {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t size;
	size_t capacity;
};

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_init initializes a SqshBuffer.
 *
 * @param[out] buffer The SqshBuffer to initialize.
 */
SQSH_NO_UNUSED int sqsh__buffer_init(struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_add_size tells SqshBuffer to increase the buffer by
 * additional_size
 *
 * Please be aware, that the buffer needs to be allocated by
 * sqsh__buffer_add_capacity before. Otherwise the function behavior is
 * undefined.
 *
 * @param[in,out] buffer The SqshBuffer to increase.
 * @param[in] additional_size The additional size to increase the buffer.
 */
SQSH_NO_UNUSED int
sqsh__buffer_add_size(struct SqshBuffer *buffer, size_t additional_size);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_add_size allocates additional memory for the SqshBuffer
 * and sets additional_buffer to the beginning of the additional memory.
 *
 * After sqsh__buffer_add_capacity has been called, the buffer will behave
 * undefined if you query data or size. In order to use the buffer again, you
 * need to call sqsh__buffer_add_size again.
 *
 * @param[in,out] buffer The SqshBuffer to free.
 * @param[in] additional_buffer The pointer to the additional memory.
 * @param[in] additional_size The size of the additional memory.
 */
SQSH_NO_UNUSED int sqsh__buffer_add_capacity(
		struct SqshBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_append
 *
 * @param[in,out] buffer The SqshBuffer to append to.
 * @param[in] source The data to append.
 * @param[in] source_size The size of the data to append.
 */
SQSH_NO_UNUSED int sqsh__buffer_append(
		struct SqshBuffer *buffer, const uint8_t *source,
		const size_t source_size);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_drain resets the buffer size to 0.
 *
 * This does not free the memory allocated by the buffer so that
 * the buffer can be reused.
 *
 * @param[in,out] buffer The SqshBuffer to drain.
 */

void sqsh__buffer_drain(struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_data returns the data of the SqshBuffer.
 * @param[in] buffer The SqshBuffer to get the data from.
 * @return a pointer to the data of the SqshBuffer.
 */
const uint8_t *sqsh__buffer_data(const struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_size returns the size of the SqshBuffer.
 * @param[in] buffer The SqshBuffer to get the size from.
 * @return the size of the SqshBuffer.
 */
size_t sqsh__buffer_size(const struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief sqsh__buffer_cleanup frees the memory managed by the SqshBuffer.
 * @param[in,out] buffer The SqshBuffer to cleanup.
 */
int sqsh__buffer_cleanup(struct SqshBuffer *buffer);

////////////////////////////////////////
// primitive/ref_count_array.c

typedef void (*sqsh_ref_count_array_cleanup_t)(void *data);

struct SqshRefCountArray {
	/**
	 * @privatesection
	 */
	void **data;
	size_t size;
	int *ref_count;
	pthread_mutex_t mutex;
	sqsh_ref_count_array_cleanup_t cleanup;
};

/**
 * @brief Initializes a reference-counted array.
 *
 * @param array The array to initialize.
 * @param size The size of the array.
 * @param cleanup The cleanup function.
 * @return 0 on success, a negative value on error.
 */
int sqsh__ref_count_array_init(
		struct SqshRefCountArray *array, size_t size,
		sqsh_ref_count_array_cleanup_t cleanup);

/**
 * @brief Sets a value in a reference-counted array.
 *
 * @param array The array to set the value in.
 * @param index The index to set the value at.
 * @param data The data to set.
 * @param span The number of indices to span. (Use 1 for a single index.)
 * @return 0 on success, a negative value on error.
 */
int sqsh__ref_count_array_set(
		struct SqshRefCountArray *array, int index, void *data, int span);

/**
 * @brief Gets the size of a reference-counted array.
 *
 * @param array The array to get the size of.
 * @return The size of the array.
 */
size_t sqsh__ref_count_array_size(struct SqshRefCountArray *array);

/**
 * @brief Retains the data at a specified index in a reference-counted array.
 *
 * @param array The array containing the data.
 * @param index The index of the data.
 * @return A pointer to the retained data.
 */
const void *
sqsh__ref_count_array_retain(struct SqshRefCountArray *array, int *index);

/**
 * @brief Releases the reference to the data at a specified index in a
 * reference-counted array.
 *
 * @param array The array containing the data.
 * @param index The index of the data.
 * @return 0 on success, a negative value on error.
 */
int sqsh__ref_count_array_release(struct SqshRefCountArray *array, int index);

/**
 * @brief Cleans up a reference-counted array.
 *
 * @param array The array to cleanup.
 * @return 0 on success, a negative value on error.
 */
int sqsh__ref_count_array_cleanup(struct SqshRefCountArray *array);

#ifdef __cplusplus
}
#endif
#endif // SQSH_PRIMITIVE_PRIVATE_H