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
 * @file         metablock_iterator.c
 */

#include "../../include/sqsh_extract_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_metablock_private.h"

#include "../../include/sqsh_error.h"
#include "../utils/utils.h"

int
sqsh__metablock_reader_init(
		struct SqshMetablockReader *reader, struct SqshArchive *sqsh,
		const uint64_t start_address, const uint64_t upper_limit) {
	int rv;
	rv = sqsh__metablock_iterator_init(
			&reader->iterator, sqsh, start_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__buffer_init(&reader->buffer);
	if (rv < 0) {
		goto out;
	}
	reader->size = 0;
	reader->data_size = 0;
	reader->offset = 0;
	reader->data = NULL;

out:
	if (rv < 0) {
		sqsh__metablock_reader_cleanup(reader);
	}
	return rv;
}

static int
metablock_extend(
		struct SqshMetablockReader *reader, sqsh_index_t offset, size_t size,
		sqsh_index_t end_offset) {
	int rv = 0;
	struct SqshMetablockIterator *iterator = &reader->iterator;
	struct SqshBuffer *buffer = &reader->buffer;

	sqsh__buffer_drain(buffer);

	const uint8_t *data = sqsh__metablock_iterator_data(iterator);
	const size_t data_size = sqsh__metablock_iterator_size(iterator);
	rv = sqsh__buffer_append(buffer, data, data_size);
	if (rv < 0) {
		goto out;
	}

	while (end_offset > sqsh__buffer_size(buffer)) {
		rv = sqsh__metablock_iterator_next(iterator);
		if (rv < 0) {
			goto out;
		}

		const uint8_t *data = sqsh__metablock_iterator_data(iterator);
		const size_t size = sqsh__metablock_iterator_size(iterator);
		rv = sqsh__buffer_append(buffer, data, size);
		if (rv < 0) {
			goto out;
		}
	}

	reader->data_size = sqsh__buffer_size(buffer);
	reader->data = sqsh__buffer_data(buffer);
	reader->offset = offset;
	reader->size = size;

out:
	return rv;
}

static int
metablock_map_next(struct SqshMetablockReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	struct SqshMetablockIterator *iterator = &reader->iterator;
	size_t skip = offset / SQSH_METABLOCK_BLOCK_SIZE;
	offset = offset % SQSH_METABLOCK_BLOCK_SIZE;
	sqsh_index_t end_offset;
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	if (skip > 1) {
		rv = sqsh__metablock_iterator_skip(iterator, skip - 1);
		if (rv < 0) {
			goto out;
		}
	}

	rv = sqsh__metablock_iterator_next(iterator);
	if (rv < 0) {
		goto out;
	}
	reader->data_size = sqsh__metablock_iterator_size(iterator);
	reader->data = sqsh__metablock_iterator_data(iterator);
	reader->offset = offset;
	reader->size = size;

	if (end_offset > reader->data_size) {
		rv = metablock_extend(reader, offset, size, end_offset);
	}
out:
	return rv;
}

int
sqsh__metablock_reader_advance(
		struct SqshMetablockReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;

	sqsh_index_t new_offset;
	if (SQSH_ADD_OVERFLOW(reader->offset, offset, &new_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	sqsh_index_t end_offset;
	if (SQSH_ADD_OVERFLOW(new_offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (new_offset <= reader->data_size) {
		reader->offset = new_offset;
		reader->size = size;
		if (end_offset > reader->data_size) {
			rv = metablock_extend(reader, new_offset, size, end_offset);
		}
	} else {
		rv = metablock_map_next(reader, new_offset, size);
	}

	return rv;
}

const uint8_t *
sqsh__metablock_reader_data(const struct SqshMetablockReader *reader) {
	return &reader->data[reader->offset];
}

size_t
sqsh__metablock_reader_size(const struct SqshMetablockReader *reader) {
	return reader->size;
}

int
sqsh__metablock_reader_cleanup(struct SqshMetablockReader *reader) {
	sqsh__metablock_iterator_cleanup(&reader->iterator);
	sqsh__buffer_cleanup(&reader->buffer);

	return 0;
}
