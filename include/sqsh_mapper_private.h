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
 * @file         sqsh_mapper_private.h
 */

#ifndef SQSH_PRIVATE_MAPPER_H
#define SQSH_PRIVATE_MAPPER_H

#include "sqsh_mapper.h"

#include <pthread.h>

#include "sqsh_primitive_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Sqsh;
struct SqshConfig;

////////////////////////////////////////
// mapper/curl_mapper.c

struct SqshCurlMapper {
	/**
	 * @privatesection
	 */
	const char *url;
	uint64_t expected_time;
	void *handle;
	uint8_t *header_cache;
	pthread_mutex_t lock;
};

struct SqshCurlMap {
	/**
	 * @privatesection
	 */
	uint8_t *data;
};

////////////////////////////////////////
// mapper/mmap_mapper.c

struct SqshMmapMapper {
	/**
	 * @privatesection
	 */
	int fd;
	long page_size;
};

struct SqshMmapMap {
	/**
	 * @privatesection
	 */
	uint8_t *data;
	size_t page_offset;
};

////////////////////////////////////////
// mapper/static_mapper.c

struct SqshStaticMapper {
	/**
	 * @privatesection
	 */
	const uint8_t *data;
};

struct SqshStaticMap {
	/**
	 * @privatesection
	 */
	const uint8_t *data;
};

////////////////////////////////////////
// mapper/mapper.c

struct SqshMapper;

struct SqshMapping {
	/**
	 * @privatesection
	 */
	struct SqshMapper *mapper;
	union {
		struct SqshMmapMap mm;
		struct SqshStaticMap sm;
		struct SqshCurlMap cl;
	} data;
};

struct SqshMemoryMapperImpl {
	/**
	 * @privatesection
	 */
	size_t block_size_hint;
	int (*init)(struct SqshMapper *mapper, const void *input, size_t *size);
	int (*mapping)(struct SqshMapping *map, sqsh_index_t offset, size_t size);
	int (*cleanup)(struct SqshMapper *mapper);
	const uint8_t *(*map_data)(const struct SqshMapping *mapping);
	int (*unmap)(struct SqshMapping *mapping);
};

struct SqshMapper {
	/**
	 * @privatesection
	 */
	const struct SqshMemoryMapperImpl *impl;
	size_t block_size;
	size_t archive_size;
	union {
		struct SqshMmapMapper mm;
		struct SqshStaticMapper sm;
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
int sqsh__mapper_init(
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
int sqsh__mapping_init(
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
size_t sqsh__mapper_size(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Retrieves the block size for a mapper.
 *
 * @param[in] mapper The mapper to retrieve the size from.
 *
 * @return The size of the input data in the mapper.
 */
size_t sqsh__mapper_block_size(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Cleans up a mapper.
 *
 * @param[in] mapper The mapper to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__mapper_cleanup(struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Retrieves the data in a mapping.
 *
 * @param[in] mapping The mapping to retrieve the data from.
 *
 * @return The data in the mapping.
 */
const uint8_t *sqsh__mapping_data(const struct SqshMapping *mapping);

/**
 * @internal
 * @memberof SqshMapping
 * @brief Unmaps a mapping.
 *
 * @param[in] mapping The mapping to unmap.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__mapping_cleanup(struct SqshMapping *mapping);

////////////////////////////////////////
// mapper/map_manager.c

struct SqshMapManager {
	/**
	 * @privatesection
	 */
	struct SqshMapper mapper;
	struct SqshLru lru;
	struct SqshSyncRcMap maps;
	pthread_mutex_t lock;
};

/**
 * Initializes a new instance of SqshMapManager.
 *
 * @param[out] manager The SqshMapManager instance to initialize.
 * @param[in] input The input to use.
 * @param[in] config The configuration of this sqsh instance.
 *
 * @return Returns 0 on success, a negative value on error.
 */
int sqsh__map_manager_init(
		struct SqshMapManager *manager, const void *input,
		const struct SqshConfig *config);

/**
 * Gets the size of the backing archive.
 *
 * @param[in] manager The SqshMapManager instance.
 *
 * @return Returns the size of a chunk.
 */
uint64_t sqsh__map_manager_size(const struct SqshMapManager *manager);

/**
 * Gets the size of a chunk.
 *
 * @param[in] manager The SqshMapManager instance.
 *
 * @return Returns the size of a chunk.
 */
size_t sqsh__map_manager_block_size(const struct SqshMapManager *manager);

/**
 * Gets the number of chunks in the file.
 *
 * @param[in] manager The SqshMapManager instance.
 *
 * @return Returns the number of chunks in the file.
 */
size_t sqsh__map_manager_block_count(const struct SqshMapManager *manager);

/**
 * Gets a map for a chunk.
 *
 * @param[in] manager The SqshMapManager instance.
 * @param[in] index The index of the chunk to map.
 * @param[in] span The number of chunks to map.
 * @param[out] target The mapping to store the result in.
 *
 * @return Returns 0 on success, a negative value on error.
 */
int sqsh__map_manager_get(
		struct SqshMapManager *manager, sqsh_index_t index, int span,
		const struct SqshMapping **target);

/**
 * Releases a map for a chunk.
 *
 * @param[in] manager The SqshMapManager instance.
 * @param[in] mapping The mapping to release.
 *
 * @return Returns 0 on success, a negative value on error.
 */
int sqsh__map_manager_release(
		struct SqshMapManager *manager, const struct SqshMapping *mapping);
/**
 * Cleans up the resources used by a SqshMapManager instance.
 *
 * @param[in] manager The SqshMapManager instance to cleanup.
 *
 * @return Returns 0 on success, a negative value on error.
 */
int sqsh__map_manager_cleanup(struct SqshMapManager *manager);

////////////////////////////////////////
// mapper/cursor.c

struct SqshMapCursor {
	/**
	 * @privatesection
	 */
	uint64_t address;
	uint64_t end_address;
	uint64_t upper_limit;
	struct SqshMapManager *map_manager;
	const struct SqshMapping *current_mapping;
	struct SqshBuffer buffer;
	const uint8_t *target;
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
		struct SqshMapCursor *cursor, struct SqshMapManager *mapper,
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
#endif // SQSH_PRIVATE_MAPPER_H
