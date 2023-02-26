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

int
sqsh__rc_hash_map_init(
		struct SqshRcHashMap *hash_map, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	hash_map->keys = calloc(size, sizeof(sqsh_rc_map_key_t));

	return sqsh__rc_map_init(&hash_map->values, size, element_size, cleanup);
}

const void *
sqsh__rc_hash_map_put(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key, void *data) {
	sqsh_index_t index = key_to_index(key, hash_map->values.size);
	struct SqshRcMap *values = &hash_map->values;

	const size_t size = sqsh__rc_map_size(values);
	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		if (sqsh__rc_map_is_empty(values, index) ||
			hash_map->keys[index] == key) {
			hash_map->keys[index] = key;
			return sqsh__rc_map_set(values, index, data, 1);
		}
	}

	return NULL;
}

size_t
sqsh__rc_hash_map_size(const struct SqshRcHashMap *hash_map) {
	return sqsh__rc_map_size(&hash_map->values);
}

const void *
sqsh__rc_hash_map_retain(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	struct SqshRcMap *values = &hash_map->values;
	sqsh_index_t index = key_to_index(key, hash_map->values.size);

	const size_t size = sqsh__rc_map_size(values);
	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		if (hash_map->keys[index] == key) {
			int real_index = index;
			return sqsh__rc_map_retain(values, &real_index);
		}
	}

	return NULL;
}

int
sqsh__rc_hash_map_release(struct SqshRcHashMap *hash_map, const void *element) {
	return sqsh__rc_map_release(&hash_map->values, element);
}

int
sqsh__rc_hash_map_release_key(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	struct SqshRcMap *values = &hash_map->values;
	sqsh_index_t index = key_to_index(key, hash_map->values.size);

	const size_t size = sqsh__rc_map_size(values);
	for (sqsh_index_t i = 0; i < size; i++, index++) {
		index = index % size;

		if (hash_map->keys[index] == key) {
			return sqsh__rc_map_release_index(values, index);
		}
	}

	return 0;
}

int
sqsh__rc_hash_map_cleanup(struct SqshRcHashMap *hash_map) {
	free(hash_map->keys);

	return sqsh__rc_map_cleanup(&hash_map->values);
}
