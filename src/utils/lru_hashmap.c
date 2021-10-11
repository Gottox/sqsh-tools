/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lru_hashmap
 * @created     : Friday Oct 08, 2021 20:24:58 CEST
 */

#include "lru_hashmap.h"
#include "../error.h"

#include <stdint.h>
#include <stdlib.h>

static off_t
lru_hash_to_start_index(struct SquashLruHashMap *hashmap, uint64_t hash) {
	return (hash * hash) % hashmap->size;
}

int
squash_lru_hashmap_init(struct SquashLruHashMap *hashmap, size_t size,
		SquashLruHashmapDtor dtor) {
	hashmap->dtor = dtor;
	hashmap->size = size;
	hashmap->newest = NULL;
	hashmap->oldest = NULL;
	hashmap->entries = NULL;

	hashmap->entries = calloc(size, sizeof(struct SquashLruEntry));
	if (hashmap->entries == NULL) {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
	return 0;
}
int
squash_lru_hashmap_put(
		struct SquashLruHashMap *hashmap, uint64_t hash, void *pointer) {
	off_t start_index = lru_hash_to_start_index(hashmap, hash);
	struct SquashLruEntry *candidate = NULL;

	for (off_t i = 0; i < hashmap->size; i++) {
		off_t index = (start_index + i) % hashmap->size;
		candidate = &hashmap->entries[index];

		if (candidate->pointer == NULL || candidate->pointer == pointer) {
			break;
		}
	}

	if (candidate->pointer != NULL) {
		if (hashmap->oldest == NULL) {
			// Should never happen:
			return -SQUASH_ERROR_HASHMAP_INTERNAL_ERROR;
		}

		// TODO: This is potentional slow. Instead find the current first match
		// and switch places with this one.
		candidate = hashmap->oldest;
		if (hashmap->oldest->newer) {
			hashmap->oldest->newer->older = NULL;
		}
		hashmap->oldest = hashmap->oldest->newer;
		hashmap->dtor(candidate->pointer);
	}

	if (candidate->newer) {
		candidate->newer->older = candidate->older;
	}
	if (candidate->older) {
		candidate->older->newer = candidate->newer;
	}

	candidate->pointer = pointer;
	candidate->hash = hash;
	candidate->newer = NULL;
	candidate->older = hashmap->newest;
	if (hashmap->newest) {
		hashmap->newest->newer = candidate;
	}
	hashmap->newest = candidate;

	if (hashmap->oldest == NULL) {
		hashmap->oldest = candidate;
	}

	return 0;
}
void *
squash_lru_hashmap_get(struct SquashLruHashMap *hashmap, uint64_t hash) {
	off_t start_index = lru_hash_to_start_index(hashmap, hash);

	for (off_t i = 0; i < hashmap->size; i++) {
		off_t index = (start_index + i) % hashmap->size;
		struct SquashLruEntry *candidate = &hashmap->entries[index];

		if (candidate->hash != hash) {
			continue;
		}

		return candidate->pointer;
	}

	return NULL;
}

int
squash_lru_hashmap_cleanup(struct SquashLruHashMap *hashmap) {
	for (int i = 0; i < hashmap->size; i++) {
		if (hashmap->entries[i].pointer != NULL) {
			hashmap->dtor(&hashmap->entries[i]);
		}
	}
	free(hashmap->entries);
	return 0;
}
