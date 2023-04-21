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
 * @file         reader.c
 */

#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_inode_private.h"
#include "../utils/utils.h"

int
sqsh__reader_init(
		struct SqshReader *reader, const struct SqshIteratorImpl *impl,
		void *iterator) {
	int rv;
	rv = sqsh__buffer_init(&reader->buffer);
	if (rv < 0) {
		return rv;
	}
	reader->impl = impl;
	reader->iterator = iterator;
	reader->size = 0;
	reader->data_size = 0;
	reader->offset = 0;
	reader->data = NULL;

	return 0;
}

static int
reader_extend(
		struct SqshReader *reader, sqsh_index_t offset, size_t size,
		sqsh_index_t end_offset) {
	int rv = 0;
	const struct SqshIteratorImpl *impl = reader->impl;
	void *iterator = reader->iterator;
	struct SqshBuffer *buffer = &reader->buffer;

	sqsh__buffer_drain(buffer);

	const uint8_t *data = impl->data(iterator);
	const size_t data_size = impl->size(iterator);
	rv = sqsh__buffer_append(buffer, data, data_size);
	if (rv < 0) {
		goto out;
	}

	while (end_offset > sqsh__buffer_size(buffer)) {
		rv = impl->next(iterator, 1);
		if (rv < 0) {
			goto out;
		} else if (rv == 0) {
			rv = -SQSH_ERROR_OUT_OF_BOUNDS;
			goto out;
		}

		const uint8_t *data = impl->data(iterator);
		const size_t size = impl->size(iterator);
		rv = sqsh__buffer_append(buffer, data, size);
		if (rv < 0) {
			goto out;
		}
	}

	rv = 0;
	reader->data_size = sqsh__buffer_size(buffer);
	reader->data = sqsh__buffer_data(buffer);
	reader->offset = offset;
	reader->size = size;

out:
	return rv;
}

static int
reader_map_next(struct SqshReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	void *iterator = reader->iterator;
	const struct SqshIteratorImpl *impl = reader->impl;
	size_t block_size = impl->block_size(iterator);
	size_t skip = offset / block_size;
	offset = offset % block_size;
	sqsh_index_t end_offset;
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	// At the first iteration, we're technically *before* the first block. So
	// we need to skip one block more.
	if (reader->data == NULL) {
		skip++;
	}
	rv = impl->skip(iterator, skip, 1);
	if (rv < 0) {
		goto out;
	} else if (rv == 0) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	rv = 0;

	reader->data_size = impl->size(iterator);
	reader->data = impl->data(iterator);
	reader->offset = offset;
	reader->size = size;

	if (end_offset > reader->data_size) {
		rv = reader_extend(reader, offset, size, end_offset);
	}
out:
	return rv;
}

int
sqsh__reader_advance(
		struct SqshReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;

	sqsh_index_t new_offset;
	if (SQSH_ADD_OVERFLOW(reader->offset, offset, &new_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	sqsh_index_t end_offset;
	if (SQSH_ADD_OVERFLOW(new_offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (new_offset < reader->data_size) {
		reader->offset = new_offset;
		reader->size = size;
		if (end_offset > reader->data_size) {
			rv = reader_extend(reader, new_offset, size, end_offset);
		}
	} else {
		rv = reader_map_next(reader, new_offset, size);
	}

	return rv;
}

const uint8_t *
sqsh__reader_data(const struct SqshReader *reader) {
	return &reader->data[reader->offset];
}

size_t
sqsh__reader_size(const struct SqshReader *reader) {
	return reader->size;
}

int
sqsh__reader_cleanup(struct SqshReader *reader) {
	sqsh__buffer_cleanup(&reader->buffer);

	return 0;
}
