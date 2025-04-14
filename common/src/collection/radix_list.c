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
 * @file         compression_options_data.c
 */

#include "cextras/collection.h"
#include <sqsh_collection_private.h>

int
sqsh_radix_list_init(
		struct SqshRadixList *list, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	return cx_rc_radix_tree_init(&list->tree, element_size, cleanup);
}

void *
sqsh_radix_list_put(struct SqshRadixList *list, uint64_t key, void *value) {
	return cx_rc_radix_tree_put(&list->tree, key, value);
}

int
sqsh_radix_list_release(struct SqshRadixList *list, uint64_t key) {
	return cx_rc_radix_tree_release(&list->tree, key);
}

void *
sqsh_radix_list_retain(struct SqshRadixList *list, uint64_t key) {
	return cx_rc_radix_tree_retain(&list->tree, key);
}

void
sqsh_radix_list_retain_value(struct SqshRadixList *list, const void *value) {
	cx_rc_radix_tree_retain_value(&list->tree, value);
}

int
sqsh_radix_list_cleanup(struct SqshRadixList *list) {
	return cx_rc_radix_tree_cleanup(&list->tree);
}

static void *
lru_radix_list_retain(void *backend, uint64_t index) {
	struct SqshRadixList *list = backend;
	return cx_rc_radix_tree_retain(&list->tree, index);
}

static void
lru_radix_list_retain_value(void *backend, void *value) {
	struct SqshRadixList *list = backend;
	cx_rc_radix_tree_retain_value(&list->tree, value);
}

static int
lru_radix_list_release(void *backend, uint64_t index) {
	struct SqshRadixList *list = backend;
	return cx_rc_radix_tree_release(&list->tree, index);
}

const struct CxLruBackendImpl sqsh_lru_radix_list = {
		.retain = lru_radix_list_retain,
		.retain_value = lru_radix_list_retain_value,
		.release = lru_radix_list_release,
};
