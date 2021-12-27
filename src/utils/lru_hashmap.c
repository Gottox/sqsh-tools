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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

static off_t
hash_to_start_index(struct HsqsLruHashmap *hashmap, uint64_t hash) {
	union {
		uint64_t hash;
		uint8_t bytes[8];
	} hash_bytes = {.hash = hash}, target_bytes = {0};

	for (size_t i = 0; i < 8; i++) {
		target_bytes.bytes[0] = hash_bytes.bytes[i] ^ hash_bytes.bytes[7 - i];
	}
	return target_bytes.hash % hashmap->size;
}

static struct HsqsLruEntry *
find_entry(struct HsqsLruHashmap *hashmap, uint64_t hash, bool find_free) {
	int start_index = hash_to_start_index(hashmap, hash);
	hsqs_index_t i = 0, index = 0;
	struct HsqsLruEntry *candidate = NULL;

	for (i = 0; i < hashmap->size; i++) {
		index = (start_index + i) % hashmap->size;
		candidate = &hashmap->entries[index];

		if (candidate->pointer == NULL) {
			if (find_free) {
				return candidate;
			} else {
				return NULL;
			}
		}
		if (candidate->hash == hash && candidate->pointer != NULL) {
			return candidate;
		}
	}
	return NULL;
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

static int
lru_attach(struct HsqsLruHashmap *hashmap, struct HsqsLruEntry *entry) {
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
hsqs_lru_hashmap_init(struct HsqsLruHashmap *hashmap, size_t size) {
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
		struct HsqsLruHashmap *hashmap, uint64_t hash,
		struct HsqsRefCount *pointer) {
	struct HsqsLruEntry *candidate = find_entry(hashmap, hash, true);
	hsqs_ref_count_retain(pointer);

	if (candidate == NULL) {
		if (hashmap->oldest == NULL) {
			// Should never happen:
			return -HSQS_ERROR_HASHMAP_INTERNAL_ERROR;
		}

		// TODO: This is potentional slow. Instead find the current first match
		// and switch places with this one.
		candidate = hashmap->oldest;
	}

	if (candidate->pointer != NULL) {
		hsqs_ref_count_release(candidate->pointer);
	}
	lru_detach(hashmap, candidate);
	candidate->pointer = pointer;
	candidate->hash = hash;
	lru_attach(hashmap, candidate);

	return 0;
}

struct HsqsRefCount *
hsqs_lru_hashmap_get(struct HsqsLruHashmap *hashmap, uint64_t hash) {
	struct HsqsLruEntry *candidate = find_entry(hashmap, hash, false);

	if (candidate == NULL) {
		return NULL;
	}
	lru_detach(hashmap, candidate);
	lru_attach(hashmap, candidate);

	return candidate->pointer;
}

int
hsqs_lru_hashmap_cleanup(struct HsqsLruHashmap *hashmap) {
	for (hsqs_index_t i = 0; i < hashmap->size; i++) {
		if (hashmap->entries[i].pointer != NULL) {
			hsqs_ref_count_release(hashmap->entries[i].pointer);
		}
	}
	free(hashmap->entries);
	return 0;
}
