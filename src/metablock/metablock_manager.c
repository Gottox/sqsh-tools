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
 * @file         metablock_manager.c
 */

#include "../../include/sqsh_metablock_private.h"

#include "../../include/sqsh_error.h"

#include "../../include/sqsh_primitive_private.h"

// Calculates pow(x,y) % mod
static uint64_t
mod_power(uint64_t x, uint64_t y, uint64_t mod) {
	uint64_t res = 1;

	for (; y; y = y >> 1) {
		if (y & 1) {
			res = (res * x) % mod;
		}

		x = (x * x) % mod;
	}

	return res;
}

static bool
maybe_prime(uint64_t n) {
	static const uint64_t a = 2;

	return mod_power(a, n - 1, n) == 1;
}

static uint64_t
find_next_maybe_prime(size_t n) {
	for (; maybe_prime(n) == false; n++) {
	}

	return n;
}

static void
buffer_cleanup(void *buffer) {
	sqsh__buffer_cleanup(buffer);
}

SQSH_NO_UNUSED int
sqsh__metablock_manager_init(
		struct SqshMetablockManager *metablock_manager, size_t size) {
	int rv;
	// Give a bit of room to avoid too many key hash collisions
	size = find_next_maybe_prime(2 * size);

	rv = pthread_mutex_init(&metablock_manager->lock, NULL);
	if (rv != 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	rv = sqsh__rc_hash_map_init(
			metablock_manager->hash_map, size, sizeof(struct SqshBuffer),
			buffer_cleanup);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__lru_init(
			&metablock_manager->lru, 128, &metablock_manager->hash_map->values);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		sqsh__metablock_manager_cleanup(metablock_manager);
	}
	return rv;
}

size_t
sqsh__metablock_manager_size(struct SqshMetablockManager *metablock_manager) {
	return sqsh__rc_hash_map_size(metablock_manager->hash_map);
}

SQSH_NO_UNUSED int
sqsh__metablock_manager_get(
		struct SqshMetablockManager *metablock_manager, uint64_t offset,
		size_t size, struct SqshBuffer **target) {
	(void)metablock_manager;
	(void)offset;
	(void)size;
	(void)target;
	return -SQSH_ERROR_TODO;
}

int
sqsh__metablock_manager_release(
		struct SqshMetablockManager *metablock_manager,
		struct SqshBuffer *buffer) {
	(void)metablock_manager;
	(void)buffer;
	return -SQSH_ERROR_TODO;
}

int
sqsh__metablock_manager_cleanup(
		struct SqshMetablockManager *metablock_manager) {
	sqsh__lru_cleanup(&metablock_manager->lru);
	sqsh__rc_hash_map_cleanup(metablock_manager->hash_map);
	pthread_mutex_destroy(&metablock_manager->lock);

	return 0;
}
