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
 * @file         map_iterator.c
 */

#include <sqsh_mapper_private.h>

#include <sqsh_archive.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_extract_private.h>

static sqsh_index_t
address_to_index(const struct SqshMapIterator *iterator, uint64_t address) {
	return (sqsh_index_t)address /
			sqsh__map_manager_block_size(iterator->map_manager);
}

int
sqsh__map_iterator_init(
		struct SqshMapIterator *iterator, struct SqshMapManager *manager,
		uint64_t start_address) {
	const size_t block_size = sqsh__map_manager_block_size(manager);
	const uint64_t archive_size = sqsh__map_manager_size(manager);

	if (start_address >= archive_size) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}

	iterator->map_manager = manager;
	iterator->next_index = address_to_index(iterator, start_address);
	iterator->mapping = NULL;
	iterator->data = NULL;
	iterator->size = 0;
	iterator->segment_count = SQSH_DIVIDE_CEIL(archive_size, block_size);

	return 0;
}

int
sqsh__map_iterator_copy(
		struct SqshMapIterator *target, const struct SqshMapIterator *source) {
	int rv = 0;
	target->map_manager = source->map_manager;
	// TODO
	// rv = sqsh__map_slice_copy(target->mapping, source->mapping);
	if (rv < 0) {
		goto out;
	}
	target->mapping = NULL;
	target->next_index = source->next_index;
	target->segment_count = source->segment_count;
	if (source->data != NULL) {
		target->data = sqsh__map_slice_data(source->mapping);
		target->size = sqsh__map_slice_size(source->mapping);
	} else {
		target->data = NULL;
		target->size = 0;
	}

out:
	return rv;
}

int
sqsh__map_iterator_skip(struct SqshMapIterator *iterator, uint64_t *offset) {
	int rv = 0;
	uint64_t index;
	size_t block_size = sqsh__map_manager_block_size(iterator->map_manager);

	size_t current_size = sqsh__map_iterator_size(iterator);
	if (*offset < current_size) {
		goto out;
	}

	index = iterator->next_index + (*offset / block_size);
	if (current_size > 0) {
		/* If there is a segment currently mapped, we need don't need to skip
		 * that one, hence reduce the index by one.
		 */
		index--;
	}
	if (index > SIZE_MAX) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	iterator->next_index = (size_t)index;
	bool has_next = sqsh__map_iterator_next(iterator, &rv);
	if (rv < 0) {
		goto out;
	} else if (has_next == false) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}

	*offset %= block_size;
	if (*offset > sqsh__map_iterator_size(iterator)) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	rv = 0;
out:
	return rv;
}

bool
sqsh__map_iterator_next(struct SqshMapIterator *iterator, int *err) {
	int rv;
	bool has_next = false;

	if (iterator->next_index >= iterator->segment_count) {
		rv = 0;
		iterator->data = NULL;
		iterator->size = 0;
		goto out;
	}

	sqsh__map_manager_release(iterator->map_manager, iterator->mapping);
	rv = sqsh__map_manager_get(
			iterator->map_manager, iterator->next_index, &iterator->mapping);
	if (rv < 0) {
		goto out;
	}

	iterator->size = sqsh__map_slice_size(iterator->mapping);
	iterator->data = sqsh__map_slice_data(iterator->mapping);
	has_next = iterator->size > 0;

	iterator->next_index++;
out:
	if (err != NULL) {
		*err = rv;
	}
	return has_next;
}

const uint8_t *
sqsh__map_iterator_data(const struct SqshMapIterator *iterator) {
	return iterator->data;
}

size_t
sqsh__map_iterator_block_size(const struct SqshMapIterator *iterator) {
	return sqsh__map_manager_block_size(iterator->map_manager);
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
