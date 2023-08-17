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
 * @file         xattr_iterator.c
 */

#include "../../include/sqsh_reader_private.h"

#include "../../include/sqsh_error.h"
#include "../utils/utils.h"

#include <assert.h>

static int
reader_iterator_next(struct SqshReader2 *reader, size_t desired_size) {
	int rv = reader->iterator_impl->next(reader->iterator, desired_size);
	if (rv == 0) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
	}
	return rv;
}

/**
 * @brief      Skips the iterator until the offset is reached.
 *
 * This is a naiv implementation that just calls next until the offset is
 * reached.
 *
 * TODO: Implement a iterator specific skip function.
 *
 * @param      reader        The reader
 * @param      offset        The offset
 * @param[in]  desired_size  The desired size
 *
 * @return     0 on success, negative on error.
 */
static int
reader_iterator_skip(
		struct SqshReader2 *reader, sqsh_index_t *offset, size_t desired_size) {
	int rv = 0;
	void *iterator = reader->iterator;
	const struct SqshReader2IteratorImpl *impl = reader->iterator_impl;

	size_t current_size = impl->size(iterator);

	while (current_size <= *offset) {
		*offset -= current_size;
		rv = reader_iterator_next(reader, desired_size);
		if (rv < 0) {
			goto out;
		}
		current_size = impl->size(iterator);
	}

	rv = 0;
out:
	return rv;
}

int
sqsh__reader2_init(
		struct SqshReader2 *reader,
		const struct SqshReader2IteratorImpl *iterator_impl, void *iterator) {
	reader->data = NULL;
	reader->size = 0;
	reader->offset = 0;
	reader->iterator_offset = 0;
	reader->iterator_impl = iterator_impl;
	reader->iterator = iterator;
	return sqsh__buffer_init(&reader->buffer);
}

/**
 * @brief copies data from the iterator to the buffer until the buffer
 * reaches the desired size.
 *
 * @param      reader  The reader
 * @param      offset  The offset
 * @param      size    The size
 *
 * @return     0 on success, negative on error.
 */
static int
reader_fill_buffer(struct SqshReader2 *reader, size_t size) {
	int rv = 0;
	void *iterator = reader->iterator;
	const struct SqshReader2IteratorImpl *impl = reader->iterator_impl;
	struct SqshBuffer *buffer = &reader->buffer;
	sqsh_index_t offset = reader->offset;

	size_t buffer_size = sqsh__buffer_size(buffer);
	for (;;) {
		const uint8_t *data = impl->data(iterator);
		const size_t data_size = impl->size(iterator);
		const size_t copy_size =
				SQSH_MIN(data_size - offset, size - buffer_size);
		rv = sqsh__buffer_append(buffer, &data[offset], copy_size);
		if (rv < 0) {
			goto out;
		}

		offset = 0;
		buffer_size += copy_size;
		if (size <= buffer_size) {
			assert(size == buffer_size);
			break;
		}

		rv = reader_iterator_next(reader, size);
		if (rv < 0) {
			goto out;
		}
		reader->iterator_offset = buffer_size;
	}

	reader->offset = 0;
	reader->data = sqsh__buffer_data(buffer);
	reader->size = size;
	assert(reader->iterator_offset != 0);
out:
	return rv;
}

static int
handle_buffered(struct SqshReader2 *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	struct SqshBuffer new_buffer = {0};
	sqsh_index_t iterator_offset = reader->iterator_offset;

	struct SqshBuffer *buffer = &reader->buffer;
	const uint8_t *buffer_data = sqsh__buffer_data(buffer);
	const size_t copy_size = iterator_offset - offset;

	if (offset != 0) {
		rv = sqsh__buffer_init(&new_buffer);
		if (rv < 0) {
			goto out;
		}
		rv = sqsh__buffer_append(&new_buffer, &buffer_data[offset], copy_size);
		if (rv < 0) {
			goto out;
		}
		rv = sqsh__buffer_move(buffer, &new_buffer);
		if (rv < 0) {
			goto out;
		}
	}
	rv = reader_fill_buffer(reader, size);

out:
	return rv;
}

static int
handle_mapped(struct SqshReader2 *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	void *iterator = reader->iterator;
	const struct SqshReader2IteratorImpl *impl = reader->iterator_impl;

	if (SQSH_ADD_OVERFLOW(offset, reader->offset, &offset)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = reader_iterator_skip(reader, &offset, size);
	if (rv < 0) {
		goto out;
	}

	reader->offset = offset;
	sqsh_index_t end_offset;
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	if (end_offset <= impl->size(iterator)) {
		const uint8_t *data = impl->data(iterator);
		reader->data = &data[offset];
		reader->size = size;
	} else {
		struct SqshBuffer *buffer = &reader->buffer;
		sqsh__buffer_drain(buffer);
		rv = reader_fill_buffer(reader, size);
	}

out:
	return rv;
}

int
sqsh__reader2_advance(
		struct SqshReader2 *reader, sqsh_index_t offset, size_t size) {
	if (offset >= reader->iterator_offset) {
		offset -= reader->iterator_offset;
		reader->iterator_offset = 0;
		return handle_mapped(reader, offset, size);
	} else {
		return handle_buffered(reader, offset, size);
	}
}

const uint8_t *
sqsh__reader2_data(const struct SqshReader2 *reader) {
	return reader->data;
}

size_t
sqsh__reader2_size(const struct SqshReader2 *reader) {
	return reader->size;
}

int
sqsh__reader2_cleanup(struct SqshReader2 *reader) {
	return sqsh__buffer_cleanup(&reader->buffer);
}
