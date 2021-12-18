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
lru_hash_to_start_index(struct HsqsLruHashmap *hashmap, uint64_t hash) {
	return (hash * hash) % hashmap->size;
}

/*
static int
lru_swap(struct HsqsLruHashmap *hashmap, struct HsqsLruEntry *entry1, struct
HsqsLruEntry *entry2) { struct HsqsLruEntry *newer, *older;

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
lru_detach(struct HsqsLruHashmap *hashmap, struct HsqsLruEntry *entry) {
	struct HsqsLruEntry *tmp;

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

int
hsqs_lru_hashmap_init(
		struct HsqsLruHashmap *hashmap, size_t size, HsqsLruHashmapDtor dtor) {
	hashmap->dtor = dtor;
	hashmap->size = size;
	hashmap->newest = NULL;
	hashmap->oldest = NULL;
	hashmap->entries = NULL;

	hashmap->entries = calloc(size, sizeof(struct HsqsLruEntry));
	if (hashmap->entries == NULL) {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
	return 0;
}

int
hsqs_lru_hashmap_put(
		struct HsqsLruHashmap *hashmap, uint64_t hash, void *pointer) {
	off_t start_index = lru_hash_to_start_index(hashmap, hash);
	struct HsqsLruEntry *candidate = NULL;

	for (hsqs_index_t i = 0; i < hashmap->size; i++) {
		off_t index = (start_index + i) % hashmap->size;
		candidate = &hashmap->entries[index];

		if (candidate->pointer == NULL ||
			(candidate->pointer == pointer && hash == candidate->hash)) {
			break;
		}
	}

	if (candidate->pointer != NULL) {
		if (hashmap->oldest == NULL) {
			// Should never happen:
			return -HSQS_ERROR_HASHMAP_INTERNAL_ERROR;
		}

		// TODO: This is potentional slow. Instead find the current first match
		// and switch places with this one.
		candidate = hashmap->oldest;
		hashmap->dtor(candidate->pointer);
	}

	lru_detach(hashmap, candidate);

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
hsqs_lru_hashmap_pull(struct HsqsLruHashmap *hashmap, uint64_t hash) {
	off_t start_index = lru_hash_to_start_index(hashmap, hash);

	for (hsqs_index_t i = 0; i < hashmap->size; i++) {
		hsqs_index_t index = (start_index + i) % hashmap->size;
		struct HsqsLruEntry *candidate = &hashmap->entries[index];

		if (candidate->hash != hash) {
			continue;
		}

		// Detaching the entry from the list. The user is responsible to put the
		// element back through hsqs_lru_hashmap_put() this way the the entry
		// cannot be removed when the hashmap is full.
		lru_detach(hashmap, candidate);
		return candidate->pointer;
	}

	return NULL;
}

int
hsqs_lru_hashmap_cleanup(struct HsqsLruHashmap *hashmap) {
	for (hsqs_index_t i = 0; i < hashmap->size; i++) {
		if (hashmap->entries[i].pointer != NULL) {
			hashmap->dtor(&hashmap->entries[i]);
		}
	}
	free(hashmap->entries);
	return 0;
}
