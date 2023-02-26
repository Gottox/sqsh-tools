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
 * @file         lru.c
 */

#include <stdint.h>

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

#include <assert.h>
#include <stdio.h>

#define EMPTY_MARKER SIZE_MAX

#if 0
static void
debug_print(const struct SqshLru *lru, const char msg, sqsh_index_t ring_index) {
	sqsh_index_t backend_index = lru->items[ring_index];

	fprintf(stderr, "%clru %lu: ", msg, ring_index);
	size_t sum = 0;
	for (size_t i = 0; i < lru->size; i++) {
		sqsh_index_t cur_index = lru->items[i];
		if (cur_index == backend_index) {
			sum++;
		}
	}
	fprintf(stderr, "idx: %lu refs: %lu\n", backend_index, sum);
	fflush(stderr);
}
#else
#	define debug_print(...)
#endif

int
sqsh__lru_init(
		struct SqshLru *lru, size_t size, struct SqshSyncRcMap *backend) {
	lru->backend = backend;
	lru->size = size;
	if (size == 0) {
		return 0;
	}

	lru->items = calloc(size, sizeof(sqsh_index_t));
	if (lru->items == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
	for (size_t i = 0; i < lru->size; i++) {
		lru->items[i] = EMPTY_MARKER;
	}
	lru->ring_index = 0;

	return 0;
}

int
sqsh__lru_touch(struct SqshLru *lru, sqsh_index_t index) {
	if (lru->size == 0) {
		return 0;
	}

	sqsh_index_t ring_index = lru->ring_index;
	size_t size = lru->size;
	struct SqshSyncRcMap *backend = lru->backend;
	sqsh_index_t last_index = lru->items[ring_index];

	ring_index = (ring_index + 1) % size;

	sqsh_index_t old_index = lru->items[ring_index];

	if (old_index == index || last_index == index) {
		return 0;
	}

	debug_print(lru, '-', ring_index);
	if (old_index != EMPTY_MARKER) {
		sqsh__rc_map_release_index(backend, old_index);
	}

	lru->items[ring_index] = index;
	debug_print(lru, '+', ring_index);
	int real_index = index;
	sqsh__rc_map_retain(backend, &real_index);

	lru->ring_index = ring_index;

	return 0;
}

int
sqsh__lru_cleanup(struct SqshLru *lru) {
	for (size_t i = 0; i < lru->size; i++) {
		sqsh_index_t index = lru->items[i];
		if (index != EMPTY_MARKER) {
			sqsh__rc_map_release_index(lru->backend, index);
		}
	}
	free(lru->items);
	lru->size = 0;
	lru->items = NULL;
	return 0;
}
