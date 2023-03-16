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
#include <stdint.h>

#define COLLISION_RESERVE_BITS 2

struct SqshRcHashMapInner {
	sqsh_rc_map_key_t *keys;
	struct SqshRcMap values;
	size_t count;
};

static uint64_t
djb2_hash(const void *data, const size_t size) {
	uint64_t hash = 5381;
	const uint8_t *p = data;
	for (size_t i = 0; i < size; i++) {
		hash = ((hash << 5) + hash) + p[i];
	}
	return hash;
}

static sqsh_index_t
key_to_index(const sqsh_rc_map_key_t key, const size_t size) {
	sqsh_index_t hash = djb2_hash(&key, sizeof(uint64_t));

	// reserve the lower COLLISION_RESERVE_BITS bits for collisions.
	hash <<= COLLISION_RESERVE_BITS;
	return hash % size;
}

static int
extend_hash_map(struct SqshRcHashMap *hash_map) {
	size_t new_size;
	const sqsh_index_t last_index = hash_map->hash_map_count;
	struct SqshRcHashMapInner *hash_maps;

	if (SQSH_ADD_OVERFLOW(hash_map->hash_map_count, 1, &new_size)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	hash_map->hash_map_count = new_size;
	if (SQSH_MULT_OVERFLOW(
				new_size, sizeof(struct SqshRcHashMapInner), &new_size)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}

	hash_map->hash_maps = realloc(hash_map->hash_maps, new_size);
	hash_maps = &hash_map->hash_maps[last_index];

	hash_maps->keys = calloc(hash_map->map_size, sizeof(sqsh_rc_map_key_t));
	if (hash_maps->keys == NULL) {
		return SQSH_ERROR_MALLOC_FAILED;
	}

	return sqsh__rc_map_init(
			&hash_maps->values, hash_map->map_size, hash_map->element_size,
			hash_map->cleanup);
}

int
sqsh__rc_hash_map_init(
		struct SqshRcHashMap *hash_map, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	hash_map->hash_maps = NULL;
	hash_map->hash_map_count = 0;
	hash_map->map_size = size;
	hash_map->element_size = element_size;
	hash_map->cleanup = cleanup;
	return extend_hash_map(hash_map);
}

const void *
sqsh__rc_hash_map_put(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key, void *data) {
	int rv = 0;
	const size_t size = sqsh__rc_map_size(&hash_map->hash_maps[0].values);
	const sqsh_index_t orig_index = key_to_index(key, size);
	sqsh_index_t index = orig_index;
	struct SqshRcMap *values = NULL;
	sqsh_rc_map_key_t *keys = NULL;

	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		for (sqsh_index_t j = 0; j < hash_map->hash_map_count; j++) {
			values = &hash_map->hash_maps[j].values;
			keys = hash_map->hash_maps[j].keys;
			if (sqsh__rc_map_is_empty(values, index) || keys[index] == key) {
				goto found;
			}
		}
	}

	index = orig_index;
	rv = extend_hash_map(hash_map);
	if (rv < 0) {
		return NULL;
	}
	values = &hash_map->hash_maps[hash_map->hash_map_count - 1].values;
	keys = hash_map->hash_maps[hash_map->hash_map_count - 1].keys;

found:
	keys[index] = key;
	return sqsh__rc_map_set(values, index, data, 1);
}

size_t
sqsh__rc_hash_map_size(const struct SqshRcHashMap *hash_map) {
	return sqsh__rc_map_size(&hash_map->hash_maps[0].values) *
			hash_map->hash_map_count;
}

const void *
sqsh__rc_hash_map_retain(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	const size_t size = sqsh__rc_map_size(&hash_map->hash_maps[0].values);
	sqsh_index_t index = key_to_index(key, size);

	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		for (sqsh_index_t j = 0; j < hash_map->hash_map_count; j++) {
			struct SqshRcMap *values = &hash_map->hash_maps[j].values;
			sqsh_rc_map_key_t *keys = hash_map->hash_maps[j].keys;
			if (keys[index] == key) {
				sqsh_index_t real_index = index;
				return sqsh__rc_map_retain(values, &real_index);
			}
		}
	}

	return NULL;
}

int
sqsh__rc_hash_map_release(struct SqshRcHashMap *hash_map, const void *element) {
	for (sqsh_index_t i = 0; i < hash_map->hash_map_count; i++) {
		struct SqshRcMap *values = &hash_map->hash_maps[i].values;

		if (sqsh__rc_map_contains(values, element)) {
			return sqsh__rc_map_release(values, element);
		}
	}
	return SQSH_ERROR_TODO;
}

int
sqsh__rc_hash_map_release_key(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	const size_t size = sqsh__rc_map_size(&hash_map->hash_maps[0].values);
	sqsh_index_t index = key_to_index(key, size);

	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		for (sqsh_index_t j = 0; j < hash_map->hash_map_count; j++) {
			struct SqshRcMap *values = &hash_map->hash_maps[j].values;
			sqsh_rc_map_key_t *keys = hash_map->hash_maps[j].keys;
			if (keys[index] == key) {
				return sqsh__rc_map_release_index(values, index);
			}
		}
	}

	return 0;
}

int
sqsh__rc_hash_map_cleanup(struct SqshRcHashMap *hash_map) {
	for (sqsh_index_t i = 0; i < hash_map->hash_map_count; i++) {
		free(hash_map->hash_maps[i].keys);
		sqsh__rc_map_cleanup(&hash_map->hash_maps[i].values);
	}
	free(hash_map->hash_maps);

	return 0;
}

static const void *
lru_rc_hash_map_retain(void *backend, sqsh_index_t index) {
	return sqsh__rc_hash_map_retain(backend, index);
}

static int
lru_rc_hash_map_release(void *backend, sqsh_index_t index) {
	return sqsh__rc_hash_map_release_key(backend, index);
}

const struct SqshLruBackendImpl sqsh__lru_rc_hash_map = {
		.retain = lru_rc_hash_map_retain,
		.release = lru_rc_hash_map_release,
};
