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
 * @file         collection.h
 */

#ifndef CEXTRA_COLLECTION_H
#define CEXTRA_COLLECTION_H

#include "macro.h"
#include "types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * primitive/buffer.c
 */

/**
 * @brief The CxBuffer struct is a buffer for arbitrary data.
 *
 * The buffer takes care about resizing and freeing the memory managed by The
 * buffer.
 */
struct CxBuffer {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t size;
	size_t capacity;
};

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_init initializes a CxBuffer.
 *
 * @param[out] buffer The CxBuffer to initialize.
 *
 * @return 0 on success, less than 0 on error.
 */
CX_NO_UNUSED int cx_buffer_init(struct CxBuffer *buffer);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_add_size tells CxBuffer to increase the buffer by
 * additional_size
 *
 * Please be aware, that the buffer needs to be allocated by
 * cx_buffer_add_capacity before. Otherwise the function behavior is
 * undefined.
 *
 * @param[in,out] buffer The CxBuffer to increase.
 * @param[in] additional_size The additional size to increase the buffer.
 *
 * @return 0 on success, less than 0 on error.
 */
CX_NO_UNUSED int
cx_buffer_add_size(struct CxBuffer *buffer, size_t additional_size);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_add_size allocates additional memory for the
 * CxBuffer and sets additional_buffer to the beginning of the additional
 * memory.
 *
 * After cx_buffer_add_capacity has been called, the buffer will behave
 * undefined if you query data or size. In order to use the buffer again, you
 * need to call cx_buffer_add_size again.
 *
 * @param[in,out] buffer The CxBuffer to free.
 * @param[in] additional_buffer The pointer to the additional memory.
 * @param[in] additional_size The size of the additional memory.
 *
 * @return 0 on success, less than 0 on error.
 */
CX_NO_UNUSED int cx_buffer_add_capacity(
		struct CxBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_append
 *
 * @param[in,out] buffer The CxBuffer to append to.
 * @param[in] source The data to append.
 * @param[in] source_size The size of the data to append.
 *
 * @return 0 on success, less than 0 on error.
 */
CX_NO_UNUSED int cx_buffer_append(
		struct CxBuffer *buffer, const uint8_t *source,
		const size_t source_size);

/**
 * @internal
 * @memberof CxBuffer
 * @brief moves the data from buffer source to buffer. The source buffer will be
 * invalid after this operation.
 *
 * @param[in,out] buffer The CxBuffer to move to.
 * @param[in] source The CxBuffer to move from.
 *
 * @return 0 on success, less than 0 on error.
 */
CX_NO_UNUSED int
cx_buffer_move(struct CxBuffer *buffer, struct CxBuffer *source);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief resets the buffer size to 0.
 *
 * This does not free the memory allocated by the buffer so that
 * the buffer can be reused.
 *
 * @param[in,out] buffer The SqshBuffer to drain.
 */
void cx_buffer_drain(struct CxBuffer *buffer);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_data returns the data of the CxBuffer.
 * @param[in] buffer The CxBuffer to get the data from.
 * @return a pointer to the data of the CxBuffer.
 */
const uint8_t *cx_buffer_data(const struct CxBuffer *buffer);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_size returns the size of the CxBuffer.
 *
 * @param[in] buffer The CxBuffer to get the size from.
 *
 * @return the size of the CxBuffer.
 */
size_t cx_buffer_size(const struct CxBuffer *buffer);

/**
 * @internal
 * @memberof SqshBuffer
 * @brief cleans up the buffer and returns the data.
 * @param[in,out] buffer The SqshBuffer to unwrap.
 *
 * @return 0 on success, less than 0 on error.
 */
uint8_t *cx_buffer_unwrap(struct CxBuffer *buffer);

/**
 * @internal
 * @memberof CxBuffer
 * @brief cx_buffer_cleanup frees the memory managed by the CxBuffer.
 * @param[in,out] buffer The CxBuffer to cleanup.
 *
 * @return 0 on success, less than 0 on error.
 */
int cx_buffer_cleanup(struct CxBuffer *buffer);

/***************************************
 * primitive/rc_map.c
 */

/**
 * @brief The type of the cleanup callback function.
 */
typedef void (*sqsh_rc_map_cleanup_t)(void *data);

/**
 * @internal
 * @brief The CxRcMap struct is a reference-counted array.
 */
struct CxRcMap {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t size;
	size_t element_size;
	int *ref_count;
	sqsh_rc_map_cleanup_t cleanup;
};

/**
 * @internal
 * @memberof CxRcMap
 * @brief Initializes a reference-counted array.
 *
 * @param array The array to initialize.
 * @param size The size of the array.
 * @param element_size The size of each element.
 * @param cleanup The cleanup function.
 * @return 0 on success, a negative value on error.
 */
CX_NO_UNUSED int cx_rc_map_init(
		struct CxRcMap *array, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Tests if a reference-counted array is empty.
 * @param array The array to test.
 * @param index The index to test.
 * @return True if the array is empty, false otherwise.
 */
bool cx_rc_map_is_empty(struct CxRcMap *array, cx_index_t index);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Sets a value in a reference-counted array.
 *
 * @param array The array to set the value in.
 * @param index The index to set the value at.
 * @param data The data to set.
 * @return 0 on success, a negative value on error.
 */
const void *cx_rc_map_set(struct CxRcMap *array, cx_index_t index, void *data);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Gets the size of a reference-counted array.
 *
 * @param array The array to get the size of.
 * @return The size of the array.
 */
size_t cx_rc_map_size(const struct CxRcMap *array);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Retains the data at a specified index in a reference-counted array.
 *
 * @param array The array containing the data.
 * @param index The index of the data.
 * @return A pointer to the retained data.
 */
const void *cx_rc_map_retain(struct CxRcMap *array, cx_index_t index);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Releases the reference to the data at a specified index in a
 * reference-counted array.
 *
 * @param array The array containing the data.
 * @param element The element to release.
 * @return 0 on success, a negative value on error.
 */
int cx_rc_map_release(struct CxRcMap *array, const void *element);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Releases the reference to the data at a specified index in a
 * reference-counted array.
 *
 * @param array The array containing the data.
 * @param index The index of the data to release.
 * @return 0 on success, a negative value on error.
 */
int cx_rc_map_release_index(struct CxRcMap *array, cx_index_t index);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Checks if the element is contained in the array.
 *
 * @param array The array containing the data.
 * @param element The element to check.
 * @return True if the element is contained in the array, false otherwise.
 */
bool cx_rc_map_contains(struct CxRcMap *array, const void *element);

/**
 * @internal
 * @memberof CxRcMap
 * @brief Cleans up a reference-counted array.
 *
 * @param array The array to cleanup.
 * @return 0 on success, a negative value on error.
 */
int cx_rc_map_cleanup(struct CxRcMap *array);

/**
 * @internal
 * @memberof CxRcMap
 *
 * @brief An implementation table to use CxRcMap as a CxLruBackend.
 */
extern const struct CxLruBackendImpl cx_lru_rc_map;

/***************************************
 * primitive/rc_hash_map.c
 */

/**
 * @brief The type of the key for the hash map.
 */
typedef uint64_t sqsh_rc_map_key_t;

struct CxRcHashMapInner;

/**
 * @brief A reference-counted hash map.
 */
struct CxRcHashMap {
	/**
	 * @privatesection
	 */
	struct CxRcHashMapInner *hash_maps;
	size_t hash_map_count;
	size_t map_size;
	size_t element_size;
	sqsh_rc_map_cleanup_t cleanup;
};

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Initializes a reference-counted array.
 *
 * @param hash_map The array to initialize.
 * @param size The size of the array.
 * @param element_size The size of each element.
 * @param cleanup The cleanup function.
 * @return 0 on success, a negative value on error.
 */
CX_NO_UNUSED int cx_rc_hash_map_init(
		struct CxRcHashMap *hash_map, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Sets a value in a reference-counted array.
 *
 * @param hash_map The hash map to set the value in.
 * @param key The key to set the value at.
 * @param data The data to set.
 * @return 0 on success, a negative value on error.
 */
const void *cx_rc_hash_map_put(
		struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key, void *data);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Gets the size of a reference-counted hash map.
 *
 * @param hash_map The hash map to get the size of.
 * @return The size of the hash map.
 */
size_t cx_rc_hash_map_size(const struct CxRcHashMap *hash_map);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Retains the data at a specified key in a reference-counted hash map.
 *
 * @param hash_map The hash map containing the data.
 * @param key The key of the data.
 * @return A pointer to the retained data.
 */
const void *
cx_rc_hash_map_retain(struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Releases the reference to the data at a specified index in a
 * reference-counted hash map.
 *
 * @param hash_map The hash map containing the data.
 * @param element The element to release.
 * @return 0 on success, a negative value on error.
 */
int cx_rc_hash_map_release(struct CxRcHashMap *hash_map, const void *element);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Releases the reference to the data at a specified index in a
 * reference-counted hash map.
 *
 * @param hash_map The hash map containing the data.
 * @param key The key of the data to release.
 * @return 0 on success, a negative value on error.
 */
int
cx_rc_hash_map_release_key(struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key);

/**
 * @internal
 * @memberof CxRcHashMap
 * @brief Cleans up a reference-counted hash map.
 *
 * @param hash_map The hash map to cleanup.
 * @return 0 on success, a negative value on error.
 */
int cx_rc_hash_map_cleanup(struct CxRcHashMap *hash_map);

/**
 * @internal
 * @memberof CxRcHashMap
 *
 * @brief An implementation table to use CxRcHashap as a CxLruBackend.
 */
extern const struct CxLruBackendImpl cx_lru_rc_hash_map;

/***************************************
 * primitive/lru.c
 */

/**
 * @internal
 * @brief The connection to the backend data structure. used by the LRU cache.
 */
struct CxLruBackendImpl {
	/**
	 * @brief Function that is called to retain an element.
	 */
	const void *(*retain)(void *backend, cx_index_t id);
	/**
	 * @brief Function that is called to release an element.
	 */
	int (*release)(void *backend, cx_index_t id);
};

/**
 * @internal
 * @brief Adds LRU functionality to a backend data structure.
 */
struct CxLru {
	/**
	 * @privatesection
	 */
	void *backend;
	const struct CxLruBackendImpl *impl;
	cx_index_t *items;
	cx_index_t ring_index;
	size_t size;
};

/**
 * @internal
 * @memberof CxLru
 * @brief Initializes an LRU cache.
 *
 * @param lru The LRU cache to initialize.
 * @param size The size of the LRU cache.
 * @param impl The implementation of the backend data structure.
 * @param backend The backend to use for the LRU cache.
 * @return 0 on success, a negative value on error.
 */
CX_NO_UNUSED int cx_lru_init(
		struct CxLru *lru, size_t size, const struct CxLruBackendImpl *impl,
		void *backend);

/**
 * @internal
 * @memberof CxLru
 * @brief marks an item as recently used.
 * @param lru The LRU cache to mark the item in.
 * @param id The id of the item to touch
 * @return 0 on success, a negative value on error.
 */
CX_NO_UNUSED int cx_lru_touch(struct CxLru *lru, size_t id);

/**
 * @internal
 * @memberof CxLru
 * @brief Cleans up an LRU cache.
 *
 * @param lru The LRU cache to cleanup.
 *
 * @return 0 on success, a negative value on error.
 */
int cx_lru_cleanup(struct CxLru *lru);

/***************************************
 * collection/reader.c
 */

/**
 * @internal
 * @brief A buffer that is used to read data from a CxReader.
 */
struct CxIteratorImpl {
	/**
	 * @privatesection
	 */
	int (*next)(void *iterator, size_t desired_size);
	int (*skip)(void *iterator, size_t amount, size_t desired_size);
	size_t (*block_size)(const void *iterator);
	const uint8_t *(*data)(const void *iterator);
	size_t (*size)(const void *iterator);
};

/**
 * @internal
 * @brief A buffer that is used to read data from a CxReader.
 */
struct CxReader {
	/**
	 * @privatesection
	 */
	const struct CxIteratorImpl *impl;
	void *iterator;

	cx_index_t iterator_offset;
	cx_index_t buffer_offset;
	cx_index_t data_offset;
	size_t size;
	size_t data_size;
	struct CxBuffer buffer;
	const uint8_t *data;
};

/***************************************
 * collection/collector.c
 */

typedef int (*cx_collector_next_t)(
		void *iterator, const char **value, size_t *size);

int cx_collect(char ***target, cx_collector_next_t next, void *iterator);

#ifdef __cplusplus
}
#endif
#endif /* CEXTRA_COLLECTION_H */
