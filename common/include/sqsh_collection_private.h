/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         sqsh_collection_private.h
 */

#ifndef SQSH_COLLECTION_PRIVATE_H
#define SQSH_COLLECTION_PRIVATE_H

#include <cextras/collection.h>
#include <cextras/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SqshRadixList {
	struct CxRcRadixTree tree;
};

int sqsh_radix_list_init(
		struct SqshRadixList *tree, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup);

void *
sqsh_radix_list_put(struct SqshRadixList *tree, uint64_t key, void *value);

void *sqsh_radix_list_retain(struct SqshRadixList *tree, uint64_t key);

void
sqsh_radix_list_retain_value(struct SqshRadixList *tree, const void *value);

int sqsh_radix_list_release(struct SqshRadixList *tree, uint64_t key);

int sqsh_radix_list_cleanup(struct SqshRadixList *tree);

extern const struct CxLruBackendImpl sqsh_lru_radix_list;

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_COLLECTION_PRIVATE_H */
