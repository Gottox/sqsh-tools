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
 * @file         inode_cache.c
 */

#include "../../include/sqsh_archive_private.h"

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

int
sqsh__inode_cache_init(
		struct SqshInodeCache *cache, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

	if (sqsh_superblock_has_export_table(superblock)) {
		rv = sqsh_archive_export_table(archive, &cache->export_table);
		if (rv < 0) {
			goto out;
		}
		cache->inode_refs = NULL;
	} else {
		const uint32_t inode_count = sqsh_superblock_inode_count(superblock);

		cache->export_table = NULL;
		cache->inode_refs = calloc(inode_count, sizeof(atomic_uint_fast64_t));
		if (cache->inode_refs == NULL) {
			rv = SQSH_ERROR_MALLOC_FAILED;
			goto out;
		}
	}
out:
	return rv;
}
uint64_t
sqsh__inode_cache_get(
		const struct SqshInodeCache *cache, uint64_t inode_number) {
	uint64_t inode_ref = 0;
	if (inode_number == 0) {
		return 0;
	}
	if (cache->export_table != NULL) {
		return sqsh_export_table_resolve_inode(
				cache->export_table, inode_number, &inode_ref);
	} else {
		inode_ref = atomic_load(&cache->inode_refs[inode_number - 1]);
	}
	return inode_ref;
}

int
sqsh__inode_cache_set(
		struct SqshInodeCache *cache, uint64_t inode_number,
		uint64_t inode_ref) {
	if (inode_number == 0) {
		return 0;
	}
	if (cache->export_table != NULL) {
		return 0;
	} else {
		atomic_store(&cache->inode_refs[inode_number - 1], inode_ref);
		return 0;
	}
}

int
sqsh__inode_cache_cleanup(struct SqshInodeCache *cache) {
	free(cache->inode_refs);
	return 0;
}
