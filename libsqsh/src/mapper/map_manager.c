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

#include <assert.h>

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

	const size_t block_size = sqsh__mapper_block_size(&manager->mapper);
	const size_t block_count = sqsh__map_manager_block_count(manager);
	const size_t mapper_size = sqsh__map_manager_size(manager);
	size_t size = block_size;
	sqsh_index_t offset;
	if (SQSH_MULT_OVERFLOW(index, block_size, &offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	/* If we're retrieving the last block, we need to make sure that we don't
	 * read past the end of the file, so cap the size to the remaining bytes.
	 */
	if (index == block_count - 1 && mapper_size % block_size != 0) {
		size = mapper_size % block_size;
	}

	if (SQSH_ADD_OVERFLOW(offset, manager->archive_offset, &offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	rv = sqsh__map_slice_init(mapping, &manager->mapper, offset, size);
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
	size_t map_size;
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

	const size_t mapper_size = sqsh__mapper_size(&manager->mapper);

	if (mapper_size < archive_offset) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	map_size = SQSH_DIVIDE_CEIL(
			mapper_size - archive_offset,
			sqsh__mapper_block_size(&manager->mapper));

	manager->archive_offset = archive_offset;
	rv = cx_rc_map_init(
			&manager->maps, map_size, sizeof(struct SqshMapSlice),
			map_cleanup_cb);
	if (rv < 0) {
		goto out;
	}

	rv = cx_lru_init(&manager->lru, lru_size, &cx_lru_rc_map, &manager->maps);
out:
	if (rv < 0) {
		sqsh__map_manager_cleanup(manager);
	}
	return rv;
}

uint64_t
sqsh__map_manager_size(const struct SqshMapManager *manager) {
	return sqsh__mapper_size(&manager->mapper) - manager->archive_offset;
}

size_t
sqsh__map_manager_block_size(const struct SqshMapManager *manager) {
	return sqsh__mapper_block_size(&manager->mapper);
}

size_t
sqsh__map_manager_block_count(const struct SqshMapManager *manager) {
	return cx_rc_map_size(&manager->maps);
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

	*target = cx_rc_map_retain(&manager->maps, index);

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

		*target = cx_rc_map_set(&manager->maps, index, &mapping);
	}
	rv = cx_lru_touch(&manager->lru, index);

out:
	sqsh__mutex_unlock(&manager->lock);
	return rv;
}

int
sqsh__map_manager_release(
		struct SqshMapManager *manager, const struct SqshMapSlice *mapping) {
	if (manager == NULL) {
		return 0;
	}
	int rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}

	rv = cx_rc_map_release(&manager->maps, mapping);

	sqsh__mutex_unlock(&manager->lock);
out:
	return rv;
}

int
sqsh__map_manager_cleanup(struct SqshMapManager *manager) {
	cx_lru_cleanup(&manager->lru);
	cx_rc_map_cleanup(&manager->maps);
	sqsh__mapper_cleanup(&manager->mapper);

	sqsh__mutex_destroy(&manager->lock);

	return 0;
}
