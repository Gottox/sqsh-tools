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
 * @file         inode_map.c
 */

#include <sqsh_archive_private.h>
#include <sqsh_error.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Funfact: In older versions of this library, the inode map used `0` as the
// sentinal value for empty inodes. It turned out that this was a bad idea,
// because `0` is a valid inode_ref (even if it is an invalid inode number
// though).
//
// We fixed this by not storing the inode_ref directly, but instead storing the
// complement of the inode_ref. This way, we can use `0` as a inode_ref and
// still have `UINT64_MAX` - an unlikely inode_ref - as the sentinal value for
// empty inodes without memset()ing the inode map to all `UINT64_MAX`s.

static int
export_table_init(struct SqshInodeMap *map, struct SqshArchive *archive) {
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t inode_count = sqsh_superblock_inode_count(superblock);
	map->inode_count = inode_count;

	return sqsh_archive_export_table(archive, &map->export_table);
}

static uint64_t
export_table_get(
		const struct SqshInodeMap *map, uint32_t inode_number, int *err) {
	int rv = 0;
	uint64_t inode_ref = 0;

	if (inode_number == 0 || inode_number - 1 >= map->inode_count) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	rv = sqsh_export_table_resolve_inode(
			map->export_table, inode_number, &inode_ref);
	if (rv < 0) {
		goto out;
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	return inode_ref;
}

static int
export_table_set(
		struct SqshInodeMap *map, uint32_t inode_number, uint64_t inode_ref) {
	int rv = 0;
	uint64_t actual_ref = export_table_get(map, inode_number, &rv);

	if (rv < 0) {
		return rv;
	} else if (actual_ref != inode_ref) {
		return -SQSH_ERROR_INODE_MAP_IS_INCONSISTENT;
	}
	return 0;
}

static int
dyn_map_init(struct SqshInodeMap *map, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t inode_count = sqsh_superblock_inode_count(superblock);
	map->inode_count = inode_count;

	// TODO: for 2.0 the mutex needs to be moved into the struct instead of
	// being heap allocated. This is done because dyn_map_get needs to change
	// the state of the mutex, which is not possible for a const struct.
	map->mutex = malloc(sizeof(*map->mutex));
	if (map->mutex == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	pthread_mutex_init(map->mutex, NULL);
	map->export_table = NULL;

	cx_radix_tree_init(&map->inode_refs, sizeof(uint64_t[256]));
out:
	return rv;
}

// TODO: for 2.0 make this function accept a non-const struct.
static uint64_t
dyn_map_get(const struct SqshInodeMap *map, uint32_t inode_number, int *err) {
	int rv = 0;
	uint64_t inode_ref = 0;

	if (inode_number == 0 || inode_number - 1 >= map->inode_count) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	rv = sqsh__mutex_lock(map->mutex);
	if (rv < 0) {
		goto out;
	}

	sqsh_index_t index = inode_number - 1;
	sqsh_index_t inner_index = index & 0xff;
	sqsh_index_t outer_index = index >> 8;

	uint_fast64_t *inner_inode_refs =
			cx_radix_tree_get(&map->inode_refs, outer_index);
	if (inner_inode_refs == NULL) {
		rv = -SQSH_ERROR_NO_SUCH_ELEMENT;
		goto out;
	}
	inode_ref = ~inner_inode_refs[inner_index];
	if (inode_ref == SQSH_INODE_REF_NULL) {
		rv = -SQSH_ERROR_NO_SUCH_ELEMENT;
		inode_ref = 0;
		goto out;
	}
	rv = sqsh__mutex_unlock(map->mutex);
	if (rv < 0) {
		goto out;
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	return inode_ref;
}

static int
dyn_map_set(
		struct SqshInodeMap *map, uint32_t inode_number, uint64_t inode_ref) {
	int rv = 0;

	if (inode_ref == SQSH_INODE_REF_NULL) {
		return -SQSH_ERROR_INVALID_ARGUMENT;
	} else if (inode_number == 0 || inode_number - 1 >= map->inode_count) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}

	rv = sqsh__mutex_lock(map->mutex);
	if (rv < 0) {
		return rv;
	}

	sqsh_index_t index = inode_number - 1;
	sqsh_index_t inner_index = index & 0xff;
	sqsh_index_t outer_index = index >> 8;

	uint_fast64_t *inner_inode_refs =
			cx_radix_tree_get(&map->inode_refs, outer_index);
	if (inner_inode_refs == NULL) {
		uint64_t new_inner_inode_refs[256] = {0};
		new_inner_inode_refs[inner_index] = ~inode_ref;
		cx_radix_tree_put(&map->inode_refs, outer_index, new_inner_inode_refs);
	} else {
		const uint64_t old_value = ~inner_inode_refs[inner_index];
		inner_inode_refs[inner_index] = ~inode_ref;
		if (old_value != SQSH_INODE_REF_NULL && old_value != inode_ref) {
			rv = -SQSH_ERROR_INODE_MAP_IS_INCONSISTENT;
			goto out;
		}
	}

out:
	sqsh__mutex_unlock(map->mutex);
	return rv;
}

static int
export_table_cleanup(struct SqshInodeMap *map) {
	(void)map;
	return 0;
}

static int
dyn_map_cleanup(struct SqshInodeMap *map) {
	cx_radix_tree_cleanup(&map->inode_refs);
	pthread_mutex_destroy(map->mutex);
	free(map->mutex);
	return 0;
}

static const struct SqshInodeMapImpl export_table_impl = {
		.init = export_table_init,
		.get = export_table_get,
		.set = export_table_set,
		.cleanup = export_table_cleanup,
};

static const struct SqshInodeMapImpl dyn_map_impl = {
		.init = dyn_map_init,
		.get = dyn_map_get,
		.set = dyn_map_set,
		.cleanup = dyn_map_cleanup,
};

int
sqsh__inode_map_init(struct SqshInodeMap *map, struct SqshArchive *archive) {
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t inode_count = sqsh_superblock_inode_count(superblock);
	map->inode_count = inode_count;

	if (sqsh_superblock_has_export_table(superblock)) {
		map->impl = &export_table_impl;
	} else {
		map->impl = &dyn_map_impl;
	}

	return map->impl->init(map, archive);
}

uint64_t
sqsh_inode_map_get2(
		const struct SqshInodeMap *map, uint32_t inode_number, int *err) {
	return map->impl->get(map, inode_number, err);
}

int
sqsh_inode_map_set2(
		struct SqshInodeMap *map, uint32_t inode_number, uint64_t inode_ref) {
	return map->impl->set(map, inode_number, inode_ref);
}

uint64_t
sqsh_inode_map_get(const struct SqshInodeMap *map, uint64_t inode_number) {
	return sqsh_inode_map_get2(map, (uint32_t)inode_number, NULL);
}

int
sqsh_inode_map_set(
		struct SqshInodeMap *map, uint64_t inode_number, uint64_t inode_ref) {
	return sqsh_inode_map_set2(map, (uint32_t)inode_number, inode_ref);
}

int
sqsh__inode_map_cleanup(struct SqshInodeMap *map) {
	return map->impl->cleanup(map);
	return 0;
}
