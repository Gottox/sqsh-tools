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
get_index(const struct SqshMapCursor *cursor, uint64_t address) {
	return address / sqsh__map_manager_block_size(cursor->map_manager);
}

static sqsh_index_t
get_offset(const struct SqshMapCursor *cursor, uint64_t address) {
	return address % sqsh__map_manager_block_size(cursor->map_manager);
}

int
sqsh__map_cursor_init(
		struct SqshMapCursor *cursor, struct SqshMapManager *map_manager,
		const uint64_t start_address, const uint64_t upper_limit) {
	cursor->address = start_address;
	cursor->upper_limit = upper_limit;
	cursor->map_manager = map_manager;
	cursor->current_mapping = NULL;

	return sqsh__buffer_init(&cursor->buffer);
}

static int
setup_direct(struct SqshMapCursor *cursor) {
	return sqsh__map_manager_get(
			cursor->map_manager, get_index(cursor, cursor->address), 1,
			&cursor->current_mapping);
}

static int
add_buffered(
		struct SqshMapCursor *cursor, sqsh_index_t index, sqsh_index_t offset,
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
setup_buffered(struct SqshMapCursor *cursor) {
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

out:
	return rv;
}

int
sqsh__map_cursor_advance(
		struct SqshMapCursor *cursor, sqsh_index_t offset, size_t size) {
	int rv = 0;

	sqsh__map_manager_release(cursor->map_manager, cursor->current_mapping);
	cursor->current_mapping = NULL;

	if (SQSH_ADD_OVERFLOW(cursor->address, offset, &cursor->address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(cursor->address, size, &cursor->end_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (cursor->end_address > cursor->upper_limit) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	if (get_index(cursor, cursor->address) ==
		get_index(cursor, cursor->end_address)) {
		rv = setup_direct(cursor);
	} else {
		rv = setup_buffered(cursor);
	}

	return rv;
}

int
sqsh__map_cursor_all(struct SqshMapCursor *cursor) {
	return sqsh__map_cursor_advance(
			cursor, 0, cursor->upper_limit - cursor->address);
}

const uint8_t *
sqsh__map_cursor_data(const struct SqshMapCursor *cursor) {
	if (cursor->current_mapping != NULL) {
		sqsh_index_t offset = get_offset(cursor, cursor->address);
		return &sqsh__mapping_data(cursor->current_mapping)[offset];
	} else {
		return sqsh__buffer_data(&cursor->buffer);
	}
}

size_t
sqsh__map_cursor_size(const struct SqshMapCursor *cursor) {
	return cursor->end_address - cursor->address;
}

int
sqsh__map_cursor_cleanup(struct SqshMapCursor *cursor) {
	if (cursor->map_manager) {
		sqsh__map_manager_release(cursor->map_manager, cursor->current_mapping);
	}
	cursor->current_mapping = NULL;
	sqsh__buffer_cleanup(&cursor->buffer);
	return 0;
}
