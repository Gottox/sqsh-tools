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
 * @file         inode_map.c
 */

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_error.h"

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define EMPTY_INODE_REF UINT64_MAX

// Funfact: In older versions of this library, the inode map used `0` as the
// sentinal value for empty inodes. It turned out that this was a bad idea,
// because `0` is a valid inode_ref (even if it is an invalid inode number
// though).
//
// We fixed this by not storing the inode_ref directly, but instead storing the
// complement of the inode_ref. This way, we can use `0` as a inode_ref and
// still have `UINT64_MAX` - an unlikely inode_ref - as the sentinal value for
// empty inodes without memset()ing the inode map to all `UINT64_MAX`s.

int
sqsh_inode_map_init(struct SqshInodeMap *map, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t inode_count = sqsh_superblock_inode_count(superblock);
	memset(map, 0, sizeof(*map));
	map->inode_count = inode_count;

	if (sqsh_superblock_has_export_table(superblock)) {
		rv = sqsh_archive_export_table(archive, &map->export_table);
		if (rv < 0) {
			goto out;
		}
		map->inode_refs = NULL;
	} else {
		map->export_table = NULL;
		map->inode_refs = calloc(inode_count, sizeof(atomic_uint_fast64_t));
		if (map->inode_refs == NULL) {
			rv = -SQSH_ERROR_MALLOC_FAILED;
			goto out;
		}
	}
out:
	return rv;
}

uint64_t
sqsh_inode_map_get2(
		const struct SqshInodeMap *map, uint32_t inode_number, int *err) {
	int rv = 0;
	uint64_t inode_ref = 0;
	atomic_uint_fast64_t *inode_refs = map->inode_refs;

	if (inode_number == 0 || inode_number - 1 >= map->inode_count) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	} else if (map->export_table != NULL) {
		rv = sqsh_export_table_resolve_inode(
				map->export_table, inode_number, &inode_ref);
		if (rv < 0) {
			goto out;
		}
	} else {
		inode_ref = ~atomic_load(&inode_refs[inode_number - 1]);
		if (inode_ref == EMPTY_INODE_REF) {
			rv = -SQSH_ERROR_NO_SUCH_ELEMENT;
			inode_ref = 0;
			goto out;
		}
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	return inode_ref;
}

int
sqsh_inode_map_set2(
		struct SqshInodeMap *map, uint32_t inode_number, uint64_t inode_ref) {
	uint64_t old_value;
	atomic_uint_fast64_t *inode_refs = map->inode_refs;

	if (inode_ref == EMPTY_INODE_REF) {
		return -SQSH_ERROR_INVALID_ARGUMENT;
	} else if (inode_number == 0 || inode_number - 1 >= map->inode_count) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	} else if (inode_number != 0 && map->export_table == NULL) {
		old_value = ~atomic_exchange(&inode_refs[inode_number - 1], ~inode_ref);
		if (old_value != EMPTY_INODE_REF && old_value != inode_ref) {
			return -SQSH_ERROR_INODE_MAP_IS_INCONSISTENT;
		}
	}
	return 0;
}

uint64_t
sqsh_inode_map_get(const struct SqshInodeMap *map, uint64_t inode_number) {
	return sqsh_inode_map_get2(map, inode_number, NULL);
}

int
sqsh_inode_map_set(
		struct SqshInodeMap *map, uint64_t inode_number, uint64_t inode_ref) {
	return sqsh_inode_map_set2(map, inode_number, inode_ref);
}

int
sqsh_inode_map_cleanup(struct SqshInodeMap *map) {
	free(map->inode_refs);
	return 0;
}
