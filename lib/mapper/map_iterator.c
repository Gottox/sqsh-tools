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
 * @file         map_iterator.c
 */

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_error.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_extract_private.h"
#include "../utils/utils.h"

static size_t
address_to_index(const struct SqshMapIterator *iterator, uint64_t address) {
	return address / sqsh__map_manager_block_size(iterator->map_manager);
}

static size_t
index_to_address(const struct SqshMapIterator *iterator, uint64_t index) {
	return index * sqsh__map_manager_block_size(iterator->map_manager);
}

int
sqsh__map_iterator_init(
		struct SqshMapIterator *iterator, struct SqshMapManager *manager,
		uint64_t start_address) {
	const size_t block_size = sqsh__map_manager_block_size(manager);
	const size_t archive_size = sqsh__map_manager_size(manager);

	if (start_address >= archive_size) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}

	iterator->map_manager = manager;
	iterator->index = address_to_index(iterator, start_address);
	iterator->mapping = NULL;
	iterator->data = NULL;
	iterator->size = 0;
	iterator->segment_count = SQSH_DIVIDE_CEIL(archive_size, block_size);

	return 0;
}

int
sqsh__map_iterator_next(struct SqshMapIterator *iterator) {
	return sqsh__map_iterator_skip(iterator, 1);
}

int
sqsh__map_iterator_skip(struct SqshMapIterator *iterator, size_t amount) {
	int rv;

	if (amount == 0) {
		return 1;
	}
	iterator->index += amount;
	if (iterator->mapping == NULL) {
		iterator->index -= 1;
	}

	if (iterator->index >= iterator->segment_count) {
		rv = 0;
		iterator->data = NULL;
		iterator->size = 0;
		goto out;
	}

	sqsh__map_manager_release(iterator->map_manager, iterator->mapping);
	rv = sqsh__map_manager_get(
			iterator->map_manager, iterator->index, &iterator->mapping);
	if (rv < 0) {
		goto out;
	}

	iterator->size = rv = sqsh__map_slice_size(iterator->mapping);
	iterator->data = sqsh__map_slice_data(iterator->mapping);
out:
	return rv;
}

const uint8_t *
sqsh__map_iterator_data(const struct SqshMapIterator *iterator) {
	return iterator->data;
}

size_t
sqsh__map_iterator_block_size(const struct SqshMapIterator *iterator) {
	return sqsh__map_manager_block_size(iterator->map_manager);
}

sqsh_index_t
sqsh__map_iterator_address(const struct SqshMapIterator *iterator) {
	return index_to_address(iterator, iterator->index);
}

size_t
sqsh__map_iterator_size(const struct SqshMapIterator *iterator) {
	return iterator->size;
}

int
sqsh__map_iterator_cleanup(struct SqshMapIterator *iterator) {
	sqsh__map_manager_release(iterator->map_manager, iterator->mapping);
	iterator->mapping = NULL;
	return 0;
}
