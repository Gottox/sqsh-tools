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

static int
map_iterator_next(void *iterator, size_t desired_size) {
	(void)desired_size;
	return sqsh__map_iterator_next(iterator);
}
static int
map_iterator_skip(void *iterator, size_t amount, size_t desired_size) {
	(void)desired_size;
	return sqsh__map_iterator_skip(iterator, amount);
}
static size_t
map_iterator_block_size(const void *iterator) {
	return sqsh__map_iterator_block_size(iterator);
}
static const uint8_t *
map_iterator_data(const void *iterator) {
	return sqsh__map_iterator_data(iterator);
}
static size_t
map_iterator_size(const void *iterator) {
	return sqsh__map_iterator_size(iterator);
}

static const struct SqshIteratorImpl map_reader_impl = {
		.next = map_iterator_next,
		.skip = map_iterator_skip,
		.block_size = map_iterator_block_size,
		.data = map_iterator_data,
		.size = map_iterator_size,
};

static sqsh_index_t
get_offset(const struct SqshMapReader *reader, uint64_t address) {
	return address % sqsh__map_iterator_block_size(&reader->iterator);
}

int
sqsh__map_reader_init(
		struct SqshMapReader *reader, struct SqshMapManager *map_manager,
		uint64_t start_address, const uint64_t upper_limit) {
	int rv;
	sqsh_index_t offset =
			start_address % sqsh__map_manager_block_size(map_manager);

	rv = sqsh__map_iterator_init(
			&reader->iterator, map_manager, start_address - offset);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__reader_init(
			&reader->reader, &map_reader_impl, &reader->iterator);
	if (rv < 0) {
		goto out;
	}
	reader->upper_limit = upper_limit;
	reader->address = start_address;

	rv = sqsh__reader_advance(&reader->reader, offset, 0);
out:
	return rv;
}

size_t
sqsh__map_reader_remaining_direct(const struct SqshMapReader *reader) {
	size_t block_size = sqsh__map_iterator_block_size(&reader->iterator);
	sqsh_index_t offset = get_offset(reader, reader->address);
	return block_size - offset;
}

uint64_t
sqsh__map_reader_address(const struct SqshMapReader *reader) {
	return reader->address;
}

int
sqsh__map_reader_advance(
		struct SqshMapReader *reader, sqsh_index_t offset, size_t size) {
	reader->address += offset;
	return sqsh__reader_advance(&reader->reader, offset, size);
}

int
sqsh__map_reader_all(struct SqshMapReader *reader) {
	return sqsh__map_reader_advance(
			reader, 0, reader->upper_limit - reader->address);
}

const uint8_t *
sqsh__map_reader_data(const struct SqshMapReader *reader) {
	return sqsh__reader_data(&reader->reader);
}

size_t
sqsh__map_reader_size(const struct SqshMapReader *reader) {
	return sqsh__reader_size(&reader->reader);
}

int
sqsh__map_reader_cleanup(struct SqshMapReader *reader) {
	sqsh__reader_cleanup(&reader->reader);
	sqsh__map_iterator_cleanup(&reader->iterator);

	return 0;
}
