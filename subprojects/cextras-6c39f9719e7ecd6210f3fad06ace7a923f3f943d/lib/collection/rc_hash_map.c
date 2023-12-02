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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define COLLISION_RESERVE_BITS 2
#define MAX_COLLISIONS 5

struct CxRcHashMapInner {
	sqsh_rc_map_key_t *keys;
	struct CxRcMap values;
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

static size_t
key_to_index(const sqsh_rc_map_key_t key, const size_t size) {
	size_t hash = djb2_hash(&key, sizeof(uint64_t));

	/* reserve the lower COLLISION_RESERVE_BITS bits for collisions. */
	hash <<= COLLISION_RESERVE_BITS;
	return hash % size;
}

static int
extend_hash_map(struct CxRcHashMap *hash_map) {
	size_t new_size;
	const size_t last_index = hash_map->hash_map_count;
	struct CxRcHashMapInner *hash_maps;

	if (CX_ADD_OVERFLOW(hash_map->hash_map_count, 1, &new_size)) {
		return -CX_ERR_INTEGER_OVERFLOW;
	}
	hash_map->hash_map_count = new_size;
	if (CX_MUL_OVERFLOW(new_size, sizeof(struct CxRcHashMapInner), &new_size)) {
		return -CX_ERR_INTEGER_OVERFLOW;
	}

	hash_map->hash_maps = realloc(hash_map->hash_maps, new_size);
	hash_maps = &hash_map->hash_maps[last_index];

	hash_maps->keys = calloc(hash_map->map_size, sizeof(sqsh_rc_map_key_t));
	if (hash_maps->keys == NULL) {
		return -CX_ERR_ALLOC;
	}

	return cx_rc_map_init(
			&hash_maps->values, hash_map->map_size, hash_map->element_size,
			hash_map->cleanup);
}

int
cx_rc_hash_map_init(
		struct CxRcHashMap *hash_map, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	memset(hash_map, 0, sizeof(*hash_map));
	hash_map->hash_maps = NULL;
	hash_map->hash_map_count = 0;
	hash_map->map_size = size;
	hash_map->element_size = element_size;
	hash_map->cleanup = cleanup;
	return extend_hash_map(hash_map);
}

const void *
cx_rc_hash_map_put(
		struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key, void *data) {
	int rv = 0;
	const size_t size = cx_rc_map_size(&hash_map->hash_maps[0].values);
	const size_t orig_index = key_to_index(key, size);
	size_t index = orig_index;
	struct CxRcMap *values = NULL;
	sqsh_rc_map_key_t *keys = NULL;

	for (size_t i = 0; i < size && i < MAX_COLLISIONS; i++, index++) {
		index = index % size;

		for (size_t j = 0; j < hash_map->hash_map_count; j++) {
			values = &hash_map->hash_maps[j].values;
			keys = hash_map->hash_maps[j].keys;
			if (cx_rc_map_is_empty(values, index) || keys[index] == key) {
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
	return cx_rc_map_set(values, index, data);
}

size_t
cx_rc_hash_map_size(const struct CxRcHashMap *hash_map) {
	return cx_rc_map_size(&hash_map->hash_maps[0].values) *
			hash_map->hash_map_count;
}

const void *
cx_rc_hash_map_retain(struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	const size_t size = cx_rc_map_size(&hash_map->hash_maps[0].values);
	size_t index = key_to_index(key, size);

	for (size_t i = 0; i < size && i < MAX_COLLISIONS; i++, index++) {
		index = index % size;

		for (size_t j = 0; j < hash_map->hash_map_count; j++) {
			struct CxRcMap *values = &hash_map->hash_maps[j].values;
			sqsh_rc_map_key_t *keys = hash_map->hash_maps[j].keys;
			if (keys[index] == key) {
				return cx_rc_map_retain(values, index);
			}
		}
	}

	return NULL;
}

int
cx_rc_hash_map_release(struct CxRcHashMap *hash_map, const void *element) {
	for (size_t i = 0; i < hash_map->hash_map_count; i++) {
		struct CxRcMap *values = &hash_map->hash_maps[i].values;

		if (cx_rc_map_contains(values, element)) {
			return cx_rc_map_release(values, element);
		}
	}

	return -CX_ERR_NOT_FOUND;
}

int
cx_rc_hash_map_release_key(
		struct CxRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	const size_t size = cx_rc_map_size(&hash_map->hash_maps[0].values);
	size_t index = key_to_index(key, size);

	for (size_t i = 0; i < size; i++, index++) {
		index = index % size;

		for (size_t j = 0; j < hash_map->hash_map_count; j++) {
			struct CxRcMap *values = &hash_map->hash_maps[j].values;
			sqsh_rc_map_key_t *keys = hash_map->hash_maps[j].keys;
			if (keys[index] == key) {
				return cx_rc_map_release_index(values, index);
			}
		}
	}

	return 0;
}

int
cx_rc_hash_map_cleanup(struct CxRcHashMap *hash_map) {
	for (size_t i = 0; i < hash_map->hash_map_count; i++) {
		free(hash_map->hash_maps[i].keys);
		cx_rc_map_cleanup(&hash_map->hash_maps[i].values);
	}
	free(hash_map->hash_maps);

	return 0;
}

static const void *
lru_rc_hash_map_retain(void *backend, size_t index) {
	return cx_rc_hash_map_retain(backend, index);
}

static int
lru_rc_hash_map_release(void *backend, size_t index) {
	return cx_rc_hash_map_release_key(backend, index);
}

const struct CxLruBackendImpl cx_lru_rc_hash_map = {
		.retain = lru_rc_hash_map_retain,
		.release = lru_rc_hash_map_release,
};
