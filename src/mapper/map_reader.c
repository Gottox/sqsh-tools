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
 * @file         cursor.c
 */

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

static sqsh_index_t
get_index(const struct SqshMapReader *cursor, uint64_t address) {
	return address / sqsh__map_manager_block_size(cursor->map_manager);
}

static sqsh_index_t
get_offset(const struct SqshMapReader *cursor, uint64_t address) {
	return address % sqsh__map_manager_block_size(cursor->map_manager);
}

int
sqsh__map_reader_init(
		struct SqshMapReader *cursor, struct SqshMapManager *map_manager,
		const uint64_t start_address, const uint64_t upper_limit) {
	cursor->map_manager = map_manager;
	cursor->address = start_address;
	cursor->upper_limit = upper_limit;
	cursor->current_mapping = NULL;
	cursor->target = NULL;

	return sqsh__buffer_init(&cursor->buffer);
}

static int
replace_mapping(
		struct SqshMapReader *cursor, const struct SqshMapping *mapping) {
	if (cursor->current_mapping != NULL) {
		sqsh__map_manager_release(cursor->map_manager, cursor->current_mapping);
	}
	cursor->current_mapping = mapping;

	return 0;
}

static int
setup_direct(struct SqshMapReader *cursor) {
	int rv = 0;
	const struct SqshMapping *mapping = NULL;

	rv = sqsh__map_manager_get(
			cursor->map_manager, get_index(cursor, cursor->address), 1,
			&mapping);
	if (rv < 0) {
		goto out;
	}

	replace_mapping(cursor, mapping);

	const uint8_t *data = sqsh__mapping_data(cursor->current_mapping);
	cursor->target = &data[get_offset(cursor, cursor->address)];

out:
	return rv;
}

static int
add_buffered(
		struct SqshMapReader *cursor, sqsh_index_t index, sqsh_index_t offset,
		size_t size) {
	const struct SqshMapping *mapping = NULL;
	int rv = sqsh__map_manager_get(cursor->map_manager, index, 1, &mapping);
	if (rv < 0) {
		goto out;
	}

	const uint8_t *data = sqsh__mapping_data(mapping);
	rv = sqsh__buffer_append(&cursor->buffer, &data[offset], size - offset);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__map_manager_release(cursor->map_manager, mapping);

	return rv;
}

static int
setup_buffered(struct SqshMapReader *cursor) {
	int rv = 0;
	size_t size = sqsh__map_manager_block_size(cursor->map_manager);

	sqsh__buffer_drain(&cursor->buffer);

	sqsh_index_t index = get_index(cursor, cursor->address);
	sqsh_index_t offset = get_offset(cursor, cursor->address);

	const sqsh_index_t end_index = get_index(cursor, cursor->end_address);

	for (; index < end_index; index++) {
		rv = add_buffered(cursor, index, offset, size);
		if (rv < 0) {
			goto out;
		}
		offset = 0;
	}
	size = get_offset(cursor, cursor->end_address);
	if (size != 0) {
		add_buffered(cursor, index, offset, size);
	}

	replace_mapping(cursor, NULL);

	const uint8_t *data = sqsh__buffer_data(&cursor->buffer);
	cursor->target = data;
out:
	return rv;
}

int
sqsh__map_reader_advance(
		struct SqshMapReader *cursor, sqsh_index_t offset, size_t size) {
	int rv = 0;
	uint64_t address;
	uint64_t end_address;

	if (SQSH_ADD_OVERFLOW(cursor->address, offset, &address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(address, size, &end_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (end_address > cursor->upper_limit) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	cursor->address = address;
	cursor->end_address = end_address;

	if (get_index(cursor, cursor->address) ==
		get_index(cursor, cursor->end_address)) {
		// If both addresses point to the same index they are in the same block
		// so we can access the data directly as there are no gaps.
		rv = setup_direct(cursor);
	} else {
		// If the addresses point to different indices there are gaps in the
		// data so we need to copy the data into a buffer.
		rv = setup_buffered(cursor);
	}

	return rv;
}

int
sqsh__map_reader_all(struct SqshMapReader *cursor) {
	return sqsh__map_reader_advance(
			cursor, 0, cursor->upper_limit - cursor->address);
}

const uint8_t *
sqsh__map_reader_data(const struct SqshMapReader *cursor) {
	return cursor->target;
}

size_t
sqsh__map_reader_size(const struct SqshMapReader *cursor) {
	return cursor->end_address - cursor->address;
}

int
sqsh__map_reader_cleanup(struct SqshMapReader *cursor) {
	sqsh__buffer_cleanup(&cursor->buffer);
	replace_mapping(cursor, NULL);
	return 0;
}
