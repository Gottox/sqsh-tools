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
 * @file         extract_manager.c
 */

#include <sqsh_extract_private.h>

#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>

#include <cextras/collection.h>
#include <sqsh_mapper.h>
#include <sqsh_mapper_private.h>

static void
buffer_cleanup(void *buffer) {
	cx_buffer_cleanup(buffer);
}

SQSH_NO_UNUSED int
sqsh__extract_manager_init(
		struct SqshExtractManager *manager, struct SqshArchive *archive,
		uint32_t block_size, size_t lru_size) {
	int rv;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	enum SqshSuperblockCompressionId compression_id =
			sqsh_superblock_compression_id(superblock);

	manager->extractor_impl = sqsh__extractor_impl_from_id(compression_id);
	if (manager->extractor_impl == NULL) {
		return -SQSH_ERROR_COMPRESSION_UNSUPPORTED;
	}

	rv = sqsh__mutex_init(&manager->lock);
	if (rv < 0) {
		goto out;
	}
	rv = cx_rc_radix_tree_init(
			&manager->cache, sizeof(struct CxBuffer), buffer_cleanup);
	if (rv < 0) {
		goto out;
	}
	rv = cx_lru_init(
			&manager->lru, lru_size, &cx_lru_rc_radix_tree, &manager->cache);
	if (rv < 0) {
		goto out;
	}
	manager->map_manager = sqsh_archive_map_manager(archive);

	manager->block_size = block_size;

out:
	if (rv < 0) {
		sqsh__extract_manager_cleanup(manager);
	}
	return rv;
}

static int
extract(struct SqshExtractManager *manager, const struct SqshMapReader *reader,
		struct CxBuffer *buffer) {
	int rv = 0;
	struct SqshExtractor extractor = {0};
	const struct SqshExtractorImpl *extractor_impl = manager->extractor_impl;
	const uint32_t block_size = manager->block_size;
	const size_t size = sqsh__map_reader_size(reader);

	rv = cx_buffer_init(buffer);
	if (rv < 0) {
		goto out;
	}
	const uint8_t *data = sqsh__map_reader_data(reader);

	rv = sqsh__extractor_init(&extractor, buffer, extractor_impl, block_size);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__extractor_write(&extractor, data, size);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__extractor_finish(&extractor);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		cx_buffer_cleanup(buffer);
	}
	sqsh__extractor_cleanup(&extractor);
	return rv;
}

int
sqsh__extract_manager_uncompress(
		struct SqshExtractManager *manager, const struct SqshMapReader *reader,
		const struct CxBuffer **target) {
	int rv = 0;
	bool locked = false;
	struct CxBuffer *buffer = NULL;

	rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}
	locked = true;

	const uint64_t address = sqsh__map_reader_address(reader);

	buffer = cx_rc_radix_tree_retain(&manager->cache, address);

	if (buffer == NULL) {
		struct CxBuffer tmp_buffer = {0};
		rv = sqsh__mutex_unlock(&manager->lock);
		if (rv < 0) {
			goto out;
		}
		locked = false;

		rv = extract(manager, reader, &tmp_buffer);
		if (rv < 0) {
			goto out;
		}

		rv = sqsh__mutex_lock(&manager->lock);
		if (rv < 0) {
			goto out;
		}
		locked = true;

		buffer = cx_rc_radix_tree_put(&manager->cache, address, &tmp_buffer);
	}
	rv = cx_lru_touch_value(&manager->lru, address, buffer);
	*target = buffer;

out:
	if (locked) {
		sqsh__mutex_unlock(&manager->lock);
	}
	return rv;
}

int
sqsh__extract_manager_retain_buffer(
		struct SqshExtractManager *manager, const struct CxBuffer *buffer) {
	cx_rc_radix_tree_retain_value(&manager->cache, (void *)buffer);
	return 0;
}

int
sqsh__extract_manager_release(
		struct SqshExtractManager *manager, uint64_t address) {
	int rv = sqsh__mutex_lock(&manager->lock);
	if (rv < 0) {
		goto out;
	}

	rv = cx_rc_radix_tree_release(&manager->cache, address);

	sqsh__mutex_unlock(&manager->lock);
out:
	return rv;
}

int
sqsh__extract_manager_cleanup(struct SqshExtractManager *manager) {
	cx_lru_cleanup(&manager->lru);
	cx_rc_radix_tree_cleanup(&manager->cache);
	sqsh__mutex_destroy(&manager->lock);

	return 0;
}
