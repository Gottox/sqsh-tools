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
 * @file         map_manager.c
 */

#include <sqsh_mapper_private.h>

#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>

static void
map_cleanup_cb(void *data) {
	struct SqshMapSlice *mapping = data;
	sqsh__map_slice_cleanup(mapping);
}

SQSH_NO_UNUSED static int
load_mapping(
		struct SqshMapSlice *mapping, struct SqshMapManager *manager,
		sqsh_index_t index) {
	int rv = 0;

	const size_t block_size = sqsh_mapper_block_size(&manager->mapper);
	const uint64_t block_count = manager->block_count;
	const uint64_t mapper_size = sqsh__map_manager_size(manager);
	size_t size = block_size;
	uint64_t offset;
	if (SQSH_MULT_OVERFLOW(index, block_size, &offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	/* If we're retrieving the last block, we need to make sure that we don't
	 * read past the end of the file, so cap the size to the remaining bytes.
	 */
	if (index == block_count - 1 && mapper_size % block_size != 0) {
		size = (size_t)mapper_size % block_size;
	}

	if (SQSH_ADD_OVERFLOW(offset, manager->archive_offset, &offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	rv = sqsh__map_slice_init(mapping, &manager->mapper, index, offset, size);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh__map_manager_init(
		struct SqshMapManager *manager, const void *input,
		const struct SqshConfig *config) {
	int rv;
	const size_t lru_size = SQSH_CONFIG_DEFAULT(config->mapper_lru_size, 32);
	const uint64_t archive_offset = config->archive_offset;

	rv = sqsh__mutex_init(&manager->lock);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__mapper_init(&manager->mapper, input, config);
	if (rv < 0) {
		goto out;
	}

	const uint64_t mapper_size = sqsh_mapper_size2(&manager->mapper);

	if (mapper_size < archive_offset) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	manager->block_count = SQSH_DIVIDE_CEIL(
			mapper_size - archive_offset,
			sqsh_mapper_block_size(&manager->mapper));

	manager->archive_offset = archive_offset;
	rv = cx_rc_hash_map_init(
			&manager->maps, 1024, sizeof(struct SqshMapSlice), map_cleanup_cb);
	if (rv < 0) {
		goto out;
	}

	rv = cx_lru_init(
			&manager->lru, lru_size, &cx_lru_rc_hash_map, &manager->maps);
out:
	if (rv < 0) {
		sqsh__map_manager_cleanup(manager);
	}
	return rv;
}

uint64_t
sqsh__map_manager_size(const struct SqshMapManager *manager) {
	return sqsh_mapper_size2(&manager->mapper) - manager->archive_offset;
}

size_t
sqsh__map_manager_block_size(const struct SqshMapManager *manager) {
	return sqsh_mapper_block_size(&manager->mapper);
}

int
sqsh__map_manager_get(
		struct SqshMapManager *manager, sqsh_index_t index,
		const struct SqshMapSlice **target) {
	int rv = 0;

	rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}

	*target = cx_rc_hash_map_retain(&manager->maps, index);

	if (*target == NULL) {
		struct SqshMapSlice mapping = {0};
		sqsh__mutex_unlock(&manager->lock);
		rv = load_mapping(&mapping, manager, index);
		if (rv < 0) {
			goto out;
		}

		rv = sqsh__mutex_lock(&manager->lock);
		if (rv < 0) {
			goto out;
		}

		*target = cx_rc_hash_map_put(&manager->maps, index, &mapping);
	}
	rv = cx_lru_touch(&manager->lru, index);

out:
	sqsh__mutex_unlock(&manager->lock);
	return rv;
}

int
sqsh__map_manager_retain(
		struct SqshMapManager *manager, const struct SqshMapSlice *mapping) {
	if (manager == NULL || mapping == NULL) {
		return 0;
	}
	int rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}

	cx_rc_hash_map_retain(&manager->maps, mapping->index);

	sqsh__mutex_unlock(&manager->lock);
out:
	return rv;
}

int
sqsh__map_manager_release(
		struct SqshMapManager *manager, const struct SqshMapSlice *mapping) {
	if (manager == NULL || mapping == NULL) {
		return 0;
	}
	int rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}

	cx_rc_hash_map_release_key(&manager->maps, mapping->index);

	sqsh__mutex_unlock(&manager->lock);
out:
	return rv;
}

int
sqsh__map_manager_cleanup(struct SqshMapManager *manager) {
	cx_lru_cleanup(&manager->lru);
	cx_rc_hash_map_cleanup(&manager->maps);
	sqsh__mapper_cleanup(&manager->mapper);

	sqsh__mutex_destroy(&manager->lock);

	return 0;
}
