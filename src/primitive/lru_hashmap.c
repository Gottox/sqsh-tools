/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         lru_hashmap.c
 */

#include <sqsh_error.h>
#include <sqsh_primitive.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SQSH_LRU_HASHMAP_DEBUG
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_MISS(h) \
	{ h->misses++; }
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_HIT(h) \
	{ h->misses++; }
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_COLLISION(h) \
	{ h->collisions++; }
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_OVERFLOW(h) \
	{ h->collisions++; }
#else
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_MISS(h)
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_HIT(h)
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_COLLISION(h)
#define SQSH_LRU_HASHMAP_DEBUG_INCREASE_OVERFLOW(h)
#endif

static uint32_t
hash_to_start_index(struct SqshLruHashmap *hashmap, uint64_t hash) {
	union {
		uint64_t hash;
		uint8_t bytes[8];
	} hash_bytes = {.hash = hash}, target_bytes = {0};

	for (size_t i = 0; i < sizeof(uint64_t); i++) {
		target_bytes.bytes[i] = hash_bytes.bytes[i] ^
				hash_bytes.bytes[sizeof(uint64_t) - i - 1];
	}
	// reserve the lower 2 bits for collisions.
	target_bytes.hash <<= 2;
	return target_bytes.hash % hashmap->size;
}

static struct SqshLruEntry *
find_entry(struct SqshLruHashmap *hashmap, uint64_t hash, bool find_free) {
	uint32_t start_index = hash_to_start_index(hashmap, hash);
	sqsh_index_t i = 0, index = 0;
	struct SqshLruEntry *candidate = NULL;

	for (i = 0; i < hashmap->size; i++) {
		index = (start_index + i) % hashmap->size;
		candidate = &hashmap->entries[index];

		if (candidate->pointer == NULL) {
			if (find_free) {
				return candidate;
			} else {
				SQSH_LRU_HASHMAP_DEBUG_INCREASE_MISS(hashmap);
				return NULL;
			}
		}
		if (candidate->hash == hash) {
			SQSH_LRU_HASHMAP_DEBUG_INCREASE_HIT(hashmap);
			return candidate;
		}
		SQSH_LRU_HASHMAP_DEBUG_INCREASE_COLLISION(hashmap);
	}
	return NULL;
}

/*
static int
lru_swap(struct SqshLruHashmap *hashmap, struct SqshLruEntry *entry1, struct
SqshLruEntry *entry2) { struct SqshLruEntry *newer, *older;

	newer = entry1->newer;
	older = entry1->older;
	entry1->older = entry2->older;
	entry1->newer = entry2->newer;
	entry2->older = older;
	entry2->newer = newer;

	if (entry1 == hashmap->oldest) {
		hashmap->oldest = entry2;
	} else if (entry2 == hashmap->oldest) {
		hashmap->oldest = entry1;
	}
	if (entry1 == hashmap->newest) {
		hashmap->newest = entry2;
	} else if (entry2 == hashmap->newest) {
		hashmap->newest = entry1;
	}
	return 0;
}
*/

static int
lru_detach(struct SqshLruHashmap *hashmap, struct SqshLruEntry *entry) {
	struct SqshLruEntry *tmp;

	if (entry == hashmap->newest) {
		hashmap->newest = entry->older;
	}
	if (entry == hashmap->oldest) {
		hashmap->oldest = entry->newer;
	}
	if (entry->newer) {
		tmp = entry->newer;
		tmp->older = entry->older;
	}
	if (entry->older) {
		tmp = entry->older;
		tmp->newer = entry->newer;
	}
	entry->newer = NULL;
	entry->older = NULL;
	return 0;
}

static int
lru_attach(struct SqshLruHashmap *hashmap, struct SqshLruEntry *entry) {
	entry->newer = NULL;
	entry->older = hashmap->newest;
	if (hashmap->newest) {
		hashmap->newest->newer = entry;
	}
	hashmap->newest = entry;

	if (hashmap->oldest == NULL) {
		hashmap->oldest = entry;
	}
	return 0;
}

int
sqsh_lru_hashmap_init(struct SqshLruHashmap *hashmap, size_t size) {
	int rv = 0;
	hashmap->size = size;
	hashmap->newest = NULL;
	hashmap->oldest = NULL;
	hashmap->entries = NULL;
#ifdef SQSH_LRU_HASHMAP_DEBUG
	hashmap->hits = 0;
	hashmap->misses = 0;
	hashmap->collisions = 0;
	hashmap->overflows = 0;
#endif

	rv = pthread_mutex_init(&hashmap->lock, NULL);
	if (rv != 0) {
		rv = -SQSH_ERROR_HASHMAP_INTERNAL_ERROR;
		goto out;
	}

	hashmap->entries = calloc(size, sizeof(struct SqshLruEntry));
	if (hashmap->entries == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
out:
	if (rv < 0) {
		sqsh_lru_hashmap_cleanup(hashmap);
	}
	return 0;
}

int
sqsh_lru_hashmap_put(
		struct SqshLruHashmap *hashmap, uint64_t hash,
		struct SqshRefCount *pointer) {
	pthread_mutex_lock(&hashmap->lock);
	int rv = 0;
	struct SqshLruEntry *candidate = find_entry(hashmap, hash, true);
	sqsh_ref_count_retain(pointer);

	if (candidate == NULL) {
		if (hashmap->oldest == NULL) {
			//
			// Should never happen:
			rv = -SQSH_ERROR_HASHMAP_INTERNAL_ERROR;
			goto out;
		}

		SQSH_LRU_HASHMAP_DEBUG_INCREASE_OVERFLOW(hashmap);
		// TODO: This is potentional slow. Instead find the current first match
		// and switch places with this one.
		candidate = hashmap->oldest;
	}

	if (candidate->pointer != NULL) {
		sqsh_ref_count_release(candidate->pointer, hashmap->dtor);
	}
	lru_detach(hashmap, candidate);
	candidate->pointer = pointer;
	candidate->hash = hash;
	lru_attach(hashmap, candidate);
out:
	pthread_mutex_unlock(&hashmap->lock);
	return rv;
}

struct SqshRefCount *
sqsh_lru_hashmap_get(struct SqshLruHashmap *hashmap, uint64_t hash) {
	pthread_mutex_lock(&hashmap->lock);
	struct SqshRefCount *pointer = NULL;
	struct SqshLruEntry *candidate = find_entry(hashmap, hash, false);

	if (candidate != NULL) {
		lru_detach(hashmap, candidate);
		lru_attach(hashmap, candidate);
		pointer = candidate->pointer;
	}

	pthread_mutex_unlock(&hashmap->lock);
	return pointer;
}

int
sqsh_lru_hashmap_cleanup(struct SqshLruHashmap *hashmap) {
	if (hashmap->entries) {
		for (sqsh_index_t i = 0; i < hashmap->size; i++) {
			if (hashmap->entries[i].pointer != NULL) {
				sqsh_ref_count_release(
						hashmap->entries[i].pointer, hashmap->dtor);
			}
		}
		free(hashmap->entries);
	}
#ifdef SQSH_LRU_HASHMAP_DEBUG
	fprintf(stderr, "Hashmap size:        %lu\n", hashmap->size);
	fprintf(stderr, "Hashmap collisions:  %lu\n", hashmap->collisions);
	fprintf(stderr, "Hashmap misses:      %lu\n", hashmap->misses);
	fprintf(stderr, "Hashmap hits:        %lu\n", hashmap->hits);
	fprintf(stderr, "Hashmap overflows:   %lu\n", hashmap->overflows);
	fprintf(stderr, "Hashmap hitrate:     %f\n",
			(float)hashmap->hits / (float)hashmap->misses);
#endif
	pthread_mutex_destroy(&hashmap->lock);
	return 0;
}
