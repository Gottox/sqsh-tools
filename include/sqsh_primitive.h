/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         sqsh_primitive.h
 */

#ifndef SQSH_PRIMITIVE_H

#define SQSH_PRIMITIVE_H

#include "sqsh_common.h"

#include <pthread.h>
#include <stdbool.h>

// primitive/buffer.c

/**
 * @brief The SqshBuffer struct is a buffer for arbitrary data.
 *
 * The buffer takes care about resizing and freeing the memory managed by The
 * buffer.
 */
struct SqshBuffer {
	uint8_t *data;
	size_t size;
	size_t capacity;
};

/**
 * @brief sqsh_buffer_init initializes a SqshBuffer.
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to initialize.
 */
SQSH_NO_UNUSED int sqsh_buffer_init(struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_add_size tells SqshBuffer to increase the buffer by
 * additional_size
 * @memberof SqshBuffer
 *
 * Please be aware, that the buffer needs to be allocated by
 * sqsh_buffer_add_capacity before. Otherwise the function behavior is
 * undefined.
 *
 * @param buffer The SqshBuffer to increase.
 * @param additional_size The additional size to increase the buffer.
 */
SQSH_NO_UNUSED int
sqsh_buffer_add_size(struct SqshBuffer *buffer, size_t additional_size);

/**
 * @brief sqsh_buffer_add_size allocates additional memory for the SqshBuffer
 * and sets additional_buffer to the beginning of the additional memory.
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to free.
 * @param additional_buffer The pointer to the additional memory.
 * @param additional_size The size of the additional memory.
 */
SQSH_NO_UNUSED int sqsh_buffer_add_capacity(
		struct SqshBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size);

/**
 * @brief sqsh_buffer_append
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to append to.
 * @param source The data to append.
 * @param source_size The size of the data to append.
 */
SQSH_NO_UNUSED int sqsh_buffer_append(
		struct SqshBuffer *buffer, const uint8_t *source,
		const size_t source_size);

/**
 * @brief sqsh_buffer_drain resets the buffer size to 0.
 * @memberof SqshBuffer
 *
 * This does not free the memory allocated by the buffer so that
 * the buffer can be reused.
 *
 * @param buffer The SqshBuffer to drain.
 */

void sqsh_buffer_drain(struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_data returns the data of the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to get the data from.
 * @return a pointer to the data of the SqshBuffer.
 */
const uint8_t *sqsh_buffer_data(const struct SqshBuffer *buffer);
/**
 * @brief sqsh_buffer_size returns the size of the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to get the size from.
 * @return the size of the SqshBuffer.
 */
size_t sqsh_buffer_size(const struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_cleanup frees the memory managed by the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to cleanup.
 */
int sqsh_buffer_cleanup(struct SqshBuffer *buffer);

// primitive/ref_count.c

typedef int (*sqshRefCountDtor)(void *);

/**
 * @brief A container struct for reference counting objects.
 */
struct SqshRefCount {
	size_t references;
};

/**
 * @brief Initialize a SqshRefCount struct.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @param object_size The size of the object to be reference counted.
 * reaches 0.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_ref_count_new(struct SqshRefCount **ref_count, size_t object_size);

/**
 * @brief Increment the reference count of a SqshRefCount struct.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @return pointer to the reference counted object.
 */
void *sqsh_ref_count_retain(struct SqshRefCount *ref_count);

/**
 * @brief Decrement the reference count of a SqshRefCount struct. If the
 * reference count reaches 0, the destructor function is called.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @param dtor A pointer to a function that is called when the reference count
 * @return amount of references left.
 */
int
sqsh_ref_count_release(struct SqshRefCount *ref_count, sqshRefCountDtor dtor);

// primitive/lru_hashmap.c

typedef int (*SqshLruHashmapDtor)(void *);

struct SqshLruEntry {
	struct SqshRefCount *pointer;
	struct SqshLruEntry *newer;
	struct SqshLruEntry *older;
	uint64_t hash;
};

struct SqshLruHashmap {
	size_t size;
	struct SqshLruEntry *oldest;
	struct SqshLruEntry *newest;
	struct SqshLruEntry *entries;
	pthread_mutex_t lock;
	sqshRefCountDtor dtor;
#ifdef SQSH_LRU_HASHMAP_DEBUG
	size_t collisions;
	size_t misses;
	size_t hits;
	size_t overflows;
#endif
};

SQSH_NO_UNUSED int
sqsh_lru_hashmap_init(struct SqshLruHashmap *hashmap, size_t size);
SQSH_NO_UNUSED int sqsh_lru_hashmap_put(
		struct SqshLruHashmap *hashmap, uint64_t hash,
		struct SqshRefCount *pointer);
struct SqshRefCount *
sqsh_lru_hashmap_get(struct SqshLruHashmap *hashmap, uint64_t hash);
struct SqshRefCount *
sqsh_lru_hashmap_remove(struct SqshLruHashmap *hashmap, uint64_t hash);
int sqsh_lru_hashmap_cleanup(struct SqshLruHashmap *hashmap);

#endif /* end of include guard SQSH_PRIMITIVE_H */