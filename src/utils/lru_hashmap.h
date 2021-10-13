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
 * @file        : cache
 * @created     : Friday Oct 08, 2021 20:13:02 CEST
 */

#include "../utils.h"
#include <stdint.h>
#include <sys/types.h>

#ifndef LRU_HASHMAP_H

#define LRU_HASHMAP_H

typedef int (*SquashLruHashmapDtor)(void *);

struct SquashLruEntry {
	void *pointer;
	struct SquashLruEntry *newer;
	struct SquashLruEntry *older;
	uint64_t hash;
};

struct SquashLruHashMap {
	size_t size;
	struct SquashLruEntry *oldest;
	struct SquashLruEntry *newest;
	struct SquashLruEntry *entries;
	SquashLruHashmapDtor dtor;
};

SQUASH_NO_UNUSED int squash_lru_hashmap_init(struct SquashLruHashMap *hashmap,
		size_t size, SquashLruHashmapDtor dtor);
SQUASH_NO_UNUSED int squash_lru_hashmap_put(
		struct SquashLruHashMap *hashmap, uint64_t hash, void *pointer);
void *squash_lru_hashmap_get(struct SquashLruHashMap *hashmap, uint64_t hash);
int squash_lru_hashmap_cleanup(struct SquashLruHashMap *hashmap);

#endif /* end of include guard LRU_HASHMAP_H */