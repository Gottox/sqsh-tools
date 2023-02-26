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
 * @file         rc_map.c
 */

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__rc_hash_map_init(
		struct SqshRcHashMap *hash_map, size_t size, size_t element_size,
		sqsh_rc_map_cleanup_t cleanup) {
	(void)hash_map;
	(void)size;
	(void)element_size;
	(void)cleanup;

	return 0;
}

const void *
sqsh__rc_hash_map_put(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key, void *data) {
	(void)hash_map;
	(void)key;
	(void)data;

	return NULL;
}

size_t
sqsh__rc_hash_map_size(const struct SqshRcHashMap *hash_map) {
	(void)hash_map;

	return 0;
}

const void *
sqsh__rc_hash_map_retain(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	(void)hash_map;
	(void)key;

	return NULL;
}

int
sqsh__rc_hash_map_release(struct SqshRcHashMap *hash_map, const void *element) {
	(void)hash_map;
	(void)element;

	return 0;
}

int
sqsh__rc_hash_map_release_key(
		struct SqshRcHashMap *hash_map, sqsh_rc_map_key_t key) {
	(void)hash_map;
	(void)key;

	return 0;
}

int
sqsh__rc_hash_map_cleanup(struct SqshRcHashMap *hash_map) {
	(void)hash_map;

	return 0;
}
