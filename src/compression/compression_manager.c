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
 * @file         compression_manager.c
 */

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_archive.h"
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
sqsh__compression_manager_init(
		struct SqshCompressionManager *manager, struct SqshArchive *archive,
		size_t size) {
	int rv;
	// Give a bit of room to avoid too many key hash collisions
	size = find_next_maybe_prime(2 * size);

	rv = pthread_mutex_init(&manager->lock, NULL);
	if (rv != 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	rv = sqsh__rc_hash_map_init(
			&manager->hash_map, size, sizeof(struct SqshBuffer),
			buffer_cleanup);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__lru_init(&manager->lru, 128, &manager->hash_map.values);
	if (rv < 0) {
		goto out;
	}
	manager->map_manager = sqsh_archive_map_manager(archive);

out:
	if (rv < 0) {
		sqsh__compression_manager_cleanup(manager);
	}
	return rv;
}

size_t
sqsh__compression_manager_size(struct SqshCompressionManager *manager) {
	return sqsh__rc_hash_map_size(&manager->hash_map);
}

SQSH_NO_UNUSED int
uncompress_block(
		struct SqshBuffer *buffer, struct SqshCompressionManager *manager,
		uint64_t offset, size_t size) {
	(void)manager;
	(void)offset;
	(void)size;
	(void)buffer;
	return -SQSH_ERROR_TODO;
}

int
sqsh__compression_manager_get(
		struct SqshCompressionManager *manager, uint64_t offset, size_t size,
		const struct SqshBuffer **target) {
	int rv = 0;

	rv = pthread_mutex_lock(&manager->lock);
	if (rv != 0) {
		// rv = -SQSH_ERROR_MUTEX_LOCK;
		rv = -SQSH_ERROR_TODO;
		goto out;
	}

	*target = sqsh__rc_hash_map_retain(&manager->hash_map, offset);

	if (*target == NULL) {
		struct SqshBuffer buffer = {0};
		rv = uncompress_block(&buffer, manager, offset, size);
		if (rv < 0) {
			goto out;
		}

		*target = sqsh__rc_hash_map_put(&manager->hash_map, offset, &buffer);
	}
	rv = sqsh__lru_touch(&manager->lru, *target);

out:
	pthread_mutex_unlock(&manager->lock);
	return rv;

	return -SQSH_ERROR_TODO;
}

int
sqsh__compression_manager_release(
		struct SqshCompressionManager *manager, struct SqshBuffer *buffer) {
	(void)manager;
	(void)buffer;
	return -SQSH_ERROR_TODO;
}

int
sqsh__compression_manager_cleanup(struct SqshCompressionManager *manager) {
	sqsh__lru_cleanup(&manager->lru);
	sqsh__rc_hash_map_cleanup(&manager->hash_map);
	pthread_mutex_destroy(&manager->lock);

	return 0;
}
