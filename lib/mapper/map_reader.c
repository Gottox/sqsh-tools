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
 * @file         map_reader.c
 */

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh_error.h"
#include "../utils/utils.h"

static sqsh_index_t
get_index(const struct SqshMapReader *reader, uint64_t address) {
	return address / sqsh__map_manager_block_size(reader->map_manager);
}

static sqsh_index_t
get_offset(const struct SqshMapReader *reader, uint64_t address) {
	return address % sqsh__map_manager_block_size(reader->map_manager);
}

int
sqsh__map_reader_init(
		struct SqshMapReader *reader, struct SqshMapManager *map_manager,
		const uint64_t start_address, const uint64_t upper_limit) {
	reader->map_manager = map_manager;
	reader->end_address = reader->address = start_address;
	reader->upper_limit = upper_limit;
	reader->current_mapping = NULL;
	reader->data = NULL;

	return sqsh__buffer_init(&reader->buffer);
}

static int
replace_mapping(
		struct SqshMapReader *reader, const struct SqshMapSlice *mapping) {
	if (reader->current_mapping != NULL) {
		sqsh__map_manager_release(reader->map_manager, reader->current_mapping);
	}
	reader->current_mapping = mapping;

	return 0;
}

static int
setup_direct(struct SqshMapReader *reader) {
	int rv = 0;
	const struct SqshMapSlice *mapping = NULL;

	rv = sqsh__map_manager_get(
			reader->map_manager, get_index(reader, reader->address), 1,
			&mapping);
	if (rv < 0) {
		goto out;
	}

	replace_mapping(reader, mapping);

	const uint8_t *data = sqsh__map_slice_data(reader->current_mapping);
	reader->data = &data[get_offset(reader, reader->address)];

out:
	return rv;
}

static int
add_buffered(
		struct SqshMapReader *reader, sqsh_index_t index, sqsh_index_t offset,
		size_t size) {
	const struct SqshMapSlice *mapping = NULL;
	int rv = sqsh__map_manager_get(reader->map_manager, index, 1, &mapping);
	if (rv < 0) {
		goto out;
	}

	const uint8_t *data = sqsh__map_slice_data(mapping);
	rv = sqsh__buffer_append(&reader->buffer, &data[offset], size - offset);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__map_manager_release(reader->map_manager, mapping);

	return rv;
}

static int
setup_buffered(struct SqshMapReader *reader) {
	int rv = 0;
	size_t size = sqsh__map_manager_block_size(reader->map_manager);

	sqsh__buffer_drain(&reader->buffer);

	sqsh_index_t index = get_index(reader, reader->address);
	sqsh_index_t offset = get_offset(reader, reader->address);

	const sqsh_index_t end_index = get_index(reader, reader->end_address);

	for (; index < end_index; index++) {
		rv = add_buffered(reader, index, offset, size);
		if (rv < 0) {
			goto out;
		}
		offset = 0;
	}
	size = get_offset(reader, reader->end_address);
	if (size != 0) {
		add_buffered(reader, index, offset, size);
	}

	replace_mapping(reader, NULL);

	const uint8_t *data = sqsh__buffer_data(&reader->buffer);
	reader->data = data;
out:
	return rv;
}

size_t
sqsh__map_reader_remaining_direct(const struct SqshMapReader *reader) {
	sqsh_index_t offset = get_offset(reader, reader->address);
	size_t block_size = sqsh__map_manager_block_size(reader->map_manager);
	return block_size - offset;
}

uint64_t
sqsh__map_reader_address(const struct SqshMapReader *reader) {
	return reader->address;
}

int
sqsh__map_reader_advance(
		struct SqshMapReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	uint64_t address;
	uint64_t end_address;

	if (SQSH_ADD_OVERFLOW(reader->address, offset, &address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(address, size, &end_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (end_address > reader->upper_limit) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}

	reader->address = address;
	reader->end_address = end_address;

	if (get_index(reader, reader->address) ==
		get_index(reader, reader->end_address)) {
		// If both addresses point to the same index they are in the same block
		// so we can access the data directly as there are no gaps.
		rv = setup_direct(reader);
	} else {
		// If the addresses point to different indices there are gaps in the
		// data so we need to copy the data into a buffer.
		rv = setup_buffered(reader);
	}

	return rv;
}

int
sqsh__map_reader_all(struct SqshMapReader *reader) {
	return sqsh__map_reader_advance(
			reader, 0, reader->upper_limit - reader->address);
}

const uint8_t *
sqsh__map_reader_data(const struct SqshMapReader *reader) {
	return reader->data;
}

size_t
sqsh__map_reader_size(const struct SqshMapReader *reader) {
	return reader->end_address - reader->address;
}

int
sqsh__map_reader_cleanup(struct SqshMapReader *reader) {
	sqsh__buffer_cleanup(&reader->buffer);
	replace_mapping(reader, NULL);
	memset(reader, 0, sizeof(*reader));
	return 0;
}
