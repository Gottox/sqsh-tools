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

int
sqsh__inode_map_init(struct SqshInodeMap *map, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t inode_count = sqsh_superblock_inode_count(superblock);
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
sqsh_inode_map_get(const struct SqshInodeMap *map, uint64_t inode_number) {
	int rv;
	uint64_t inode_ref = 0;
	atomic_uint_fast64_t *inode_refs = map->inode_refs;
	if (inode_number - 1 > map->inode_count) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	} else if (inode_number == 0) {
		return 0;
	} else if (map->export_table != NULL) {
		rv = sqsh_export_table_resolve_inode(
				map->export_table, inode_number, &inode_ref);
		if (rv < 0) {
			return 0;
		}
	} else {
		inode_ref = atomic_load(&inode_refs[inode_number - 1]);
	}
	return inode_ref;
}

int
sqsh_inode_map_set(
		struct SqshInodeMap *map, uint64_t inode_number, uint64_t inode_ref) {
	uint_fast64_t old_value;
	atomic_uint_fast64_t *inode_refs = map->inode_refs;

	if (inode_number - 1 > map->inode_count) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	} else if (inode_number != 0 && map->export_table == NULL) {
		old_value = atomic_exchange(&inode_refs[inode_number - 1], inode_ref);
		if (old_value != 0 && old_value != inode_ref) {
			return -SQSH_ERROR_INODE_MAP_IS_INCONSISTENT;
		}
	}
	return 0;
}

int
sqsh__inode_map_cleanup(struct SqshInodeMap *map) {
	free(map->inode_refs);
	return 0;
}
