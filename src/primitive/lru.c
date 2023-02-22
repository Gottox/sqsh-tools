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

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__lru_init(
		struct SqshLru *lru, size_t size, struct SqshSyncRcMap *backend) {
	int rv;
	lru->backend = backend;
	lru->size = size;
	lru->items = calloc(size, sizeof(sqsh_index_t));
	if (lru->items == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
	lru->ring_index = 0;
	rv = pthread_mutex_init(&lru->lock, NULL);
	if (rv != 0) {
		return -SQSH_ERROR_TODO;
	}

	return 0;
}

int
sqsh__lru_touch(struct SqshLru *lru, sqsh_index_t index) {
	pthread_mutex_lock(&lru->lock);

	if (lru->ring_index == lru->size) {
		lru->ring_index = 0;
	}

	if (index + 1 == lru->items[lru->ring_index]) {
		goto out;
	}

	if (lru->items[lru->ring_index] > 0) {
		int old_index = lru->items[lru->ring_index] - 1;
		sqsh__sync_rc_map_release_index(lru->backend, old_index);
	}

	lru->items[lru->ring_index] = index + 1;
	int real_index = index;
	sqsh__sync_rc_map_retain(lru->backend, &real_index);

	lru->ring_index++;
out:
	pthread_mutex_unlock(&lru->lock);
	return 0;
}

int
sqsh__lru_cleanup(struct SqshLru *lru) {
	for (size_t i = 0; i < lru->size; i++) {
		if (lru->items[i]) {
			int index = lru->items[i] - 1;
			sqsh__sync_rc_map_release_index(lru->backend, index);
		}
	}
	free(lru->items);
	pthread_mutex_destroy(&lru->lock);
	lru->size = 0;
	lru->items = NULL;
	return 0;
}
