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
 * @file         sqsh_mapper.h
 */

#ifndef SQSH_MAPPER_H
#define SQSH_MAPPER_H

#include "sqsh_primitive_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Sqsh;
struct SqshConfig;

////////////////////////////////////////
// mapper/canary_mapper.c

struct SqshCanaryMapper {
	/**
	 * @privatesection
	 */
	const uint8_t *data;
	size_t size;
};

struct SqshCanaryMap {
	/**
	 * @privatesection
	 */
	uint64_t offset;
	uint8_t *data;
	size_t size;
};

extern const struct SqshMemoryMapperImpl sqsh_mapper_impl_canary;

////////////////////////////////////////
// mapper/curl_mapper.c

struct SqshCurlMapper {
	/**
	 * @privatesection
	 */
	const char *url;
	uint64_t expected_time;
	uint64_t expected_size;
	void *handle;
	pthread_mutex_t handle_lock;
};

struct SqshCurlMap {
	/**
	 * @privatesection
	 */
	struct SqshBuffer buffer;
	uint64_t offset;
};

extern const struct SqshMemoryMapperImpl sqsh_mapper_impl_curl;

////////////////////////////////////////
// mapper/mmap_full_mapper.c

struct SqshMmapFullMapper {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t size;
};

struct SqshMmapFullMap {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t size;
};

extern const struct SqshMemoryMapperImpl sqsh_mapper_impl_mmap_full;

////////////////////////////////////////
// mapper/mmap_mapper.c

struct SqshMmapMapper {
	/**
	 * @privatesection
	 */
	int fd;
	long page_size;
	size_t size;
};

struct SqshMmapMap {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t offset;
	size_t page_offset;
	size_t size;
};

extern const struct SqshMemoryMapperImpl sqsh_mapper_impl_mmap;

////////////////////////////////////////
// mapper/static_mapper.c

struct SqshStaticMapper {
	/**
	 * @privatesection
	 */
	const uint8_t *data;
	size_t size;
};

struct SqshStaticMap {
	/**
	 * @privatesection
	 */
	const uint8_t *data;
	size_t size;
};

extern const struct SqshMemoryMapperImpl sqsh_mapper_impl_static;

////////////////////////////////////////
// mapper/mapper.c

struct SqshMapper;

struct SqshMapping {
	/**
	 * @privatesection
	 */
	struct SqshMapper *mapper;
	union {
		struct SqshMmapFullMap mc;
		struct SqshMmapMap mm;
		struct SqshStaticMap sm;
		struct SqshCanaryMap cn;
		struct SqshCurlMap cl;
	} data;
};

struct SqshMemoryMapperImpl {
	/**
	 * @privatesection
	 */
	size_t block_size_hint;
	int (*init)(struct SqshMapper *mapper, const void *input, size_t size);
	int (*mapping)(struct SqshMapping *map, sqsh_index_t offset, size_t size);
	size_t (*size)(const struct SqshMapper *mapper);
	int (*cleanup)(struct SqshMapper *mapper);
	const uint8_t *(*map_data)(const struct SqshMapping *mapping);
	int (*map_resize)(struct SqshMapping *mapping, size_t new_size);
	size_t (*map_size)(const struct SqshMapping *mapping);
	int (*unmap)(struct SqshMapping *mapping);
};

struct SqshMapper {
	/**
	 * @privatesection
	 */
	const struct SqshMemoryMapperImpl *impl;
	size_t block_size;
	union {
		struct SqshMmapFullMapper mc;
		struct SqshMmapMapper mm;
		struct SqshStaticMapper sm;
		struct SqshCanaryMapper cn;
		struct SqshCurlMapper cl;
	} data;
};

/**
 * @internal
 * @memberof SqshMapper
 * @brief Initializes a mapper with an implementation and input data.
 *
 * @param[out] mapper The mapper to initialize.
 * @param[in]  input The input data to map.
 * @param[in]  config The configuration of this sqsh archive.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_mapper_init(
		struct SqshMapper *mapper, const void *input,
		const struct SqshConfig *config);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Maps a portion of the input data to a mapping.
 *
 * @param[out] mapping The mapping to store the mapped data.
 * @param[in]  mapper The mapper to use for the mapping.
 * @param[in]  offset The offset in the input data to start the mapping.
 * @param[in]  size The size of the mapped data.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_mapper_map(
		struct SqshMapping *mapping, struct SqshMapper *mapper,
		sqsh_index_t offset, size_t size);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Retrieves the size of the input data in a mapper.
 *
 * @param[in] mapper The mapper to retrieve the size from.
 *
 * @return The size of the input data in the mapper.
 */
size_t sqsh_mapper_size(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Retrieves the block size for a mapper.
 *
 * @param[in] mapper The mapper to retrieve the size from.
 *
 * @return The size of the input data in the mapper.
 */
size_t sqsh_mapper_block_size(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Cleans up a mapper.
 *
 * @param[in] mapper The mapper to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_mapper_cleanup(struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Retrieves the size of a mapping.
 *
 * @param[in] mapping The mapping to retrieve the size from.
 *
 * @return The size of the mapping.
 */
size_t sqsh_mapping_size(const struct SqshMapping *mapping);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Resizes a mapping to a new size.
 *
 * @param[in] mapping The mapping to resize.
 * @param[in] new_size The new size of the mapping.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_mapping_resize(struct SqshMapping *mapping, size_t new_size);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Retrieves the data in a mapping.
 *
 * @param[in] mapping The mapping to retrieve the data from.
 *
 * @return The data in the mapping.
 */
const uint8_t *sqsh_mapping_data(const struct SqshMapping *mapping);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Unmaps a mapping.
 *
 * @param[in] mapping The mapping to unmap.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_mapping_unmap(struct SqshMapping *mapping);

////////////////////////////////////////
// mapper/cursor.c

struct SqshMapCursor {
	/**
	 * @privatesection
	 */
	sqsh_index_t offset;
	uint64_t upper_limit;
	struct SqshMapping mapping;
	struct SqshMapper *mapper;
};

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Initializes a mapping cursor
 *
 * @param cursor The cursor to initialize
 * @param mapper The mapper to use
 * @param start_address The start address of the cursor
 * @param upper_limit The upper limit of the cursor
 * @return 0 on success, negative on error
 */
int sqsh__map_cursor_init(
		struct SqshMapCursor *cursor, struct SqshMapper *mapper,
		const uint64_t start_address, uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Advances the cursor
 *
 * @param cursor The cursor to advance
 * @param offset The offset to advance to
 * @param size The size to advance
 * @return 0 on success, negative on error
 */
int sqsh__map_cursor_advance(
		struct SqshMapCursor *cursor, sqsh_index_t offset, size_t size);

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Loads all data into the cursor. This is useful for
 * tables that must reside completely in memory.
 *
 * @param cursor The cursor to advance
 * @return 0 on success, negative on error
 */
int sqsh__map_cursor_all(struct SqshMapCursor *cursor);

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Returns the current data of the cursor
 *
 * @param cursor The cursor to get the data from
 * @return The current data of the cursor
 */
const uint8_t *sqsh__map_cursor_data(const struct SqshMapCursor *cursor);

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Returns the current size of the cursor
 *
 * @param cursor The cursor to get the size from
 * @return The current size of the cursor
 */
size_t sqsh__map_cursor_size(const struct SqshMapCursor *cursor);

/**
 * @internal
 * @memberof SqshMapCursor
 * @brief Cleans up the cursor
 *
 * @param cursor The cursor to clean up
 * @return 0 on success, negative on error
 */
int sqsh__map_cursor_cleanup(struct SqshMapCursor *cursor);

#ifdef __cplusplus
}
#endif
#endif // SQSH_MAPPER_H
