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

/**
 * # Algoritm
 *
 * The reader is a simple wrapper around an arbitrary iterator. In contrast
 * to an iterator, that just presents chunks of data, the reader allows to
 * read a specific range of data. The reader will try to map the requested
 * range to the current data. If the requested range is not fully contained
 * in the current data, the reader copy the data into a buffer and presents
 * the data from the buffer.
 *
 * 1. Forward the iterator until the start of the requested range is reached.
 *
 * 2. Test if the requested range is fully contained in the current iterator
 *    block. If so, return the data directly.
 *
 * 3. If the requested range is not fully contained in the current iterator
 *    block, copy the data into a buffer until the end of the requested range
 *    is reached.
 *
 *
 * ## Scenarios:
 *
 * ### Legend
 *
 * ```
 * ##### = directly mapped data
 *
 * ===== = data in buffer
 * ```
 *
 * ### Scenario 1: direct to direct
 *
 * Before:
 * ```
 *         Start   End
 *           v      v
 * ##########################
 * ```
 * After:
 * ```
 *                Start  End
 *                  v     v
 * ##########################
 * ```
 *
 * ### Scenario 2: direct to buffer
 *
 * Before:
 * ```
 *                Start  End
 *                  v     v
 * ##########################
 * ```
 * After:
 * ```
 *                      Start  End
 *                        v     v
 *                        =======
 *                           ##########################
 * ```
 * ### Scenario 2: buffer to direct
 *
 * Before:
 * ```
 *                      Start  End
 *                        v     v
 *                        =======
 *                           ##########################
 * ```
 * After:
 * ```
 *                            Start  End
 *                              v     v
 *                           ##########################
 * ```
 *
 * ### Scenario 3: buffer over multiple blocks
 * Before:
 * ```
 *                Start  End
 *                  v     v
 * ##########################
 * ```
 * After:
 * ```
 *                      Start                           End
 *                        v                              v
 *                        ================================
 *                           ##########################
 * ```
 */

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
	reader->iterator_offset = 0;
	reader->data_offset = 0;
	reader->data_size = 0;
	reader->buffer_offset = 0;
	reader->size = 0;
	reader->data = NULL;

	return 0;
}

static int
iterator_forward_to(
		struct SqshReader *reader, sqsh_index_t offset, size_t desired_size) {
	int rv;
	const struct SqshIteratorImpl *impl = reader->impl;
	void *iterator = reader->iterator;
	const size_t block_size = impl->block_size(iterator);
	const size_t iterator_size = impl->size(iterator);
	const size_t current_end_offset = reader->iterator_offset + iterator_size;
	if (offset < current_end_offset) {
		return 0;
	}
	size_t amount = (offset - current_end_offset) / block_size;
	reader->iterator_offset = current_end_offset + amount * block_size;
	rv = impl->skip(iterator, amount + 1, desired_size);
	if (rv < 0) {
		return rv;
	} else if (rv == 0) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}

	return 0;
}

static int
map_buffered(
		struct SqshReader *reader, sqsh_index_t new_offset,
		size_t new_end_offset) {
	int rv;
	struct SqshBuffer new_buffer = {0};
	const sqsh_index_t buffer_offset = reader->buffer_offset;
	const sqsh_index_t iterator_offset = reader->iterator_offset;
	const size_t target_size = new_end_offset - new_offset;
	const struct SqshIteratorImpl *impl = reader->impl;
	void *iterator = reader->iterator;

	rv = sqsh__buffer_init(&new_buffer);
	if (rv < 0) {
		goto out;
	}
	if (new_offset < buffer_offset) {
		// Should never happen
		abort();
	}

	/*
	 * 1. If the requested range is covered by the old buffer, copy
	 *    the data starting from the old buffer to the new buffer.
	 */
	if (new_offset < iterator_offset && buffer_offset < iterator_offset) {
		sqsh_index_t end_offset = SQSH_MIN(new_end_offset, iterator_offset);

		size_t size = end_offset - new_offset;
		sqsh_index_t inner_offset = new_offset - buffer_offset;
		const uint8_t *data = sqsh__buffer_data(&reader->buffer);

		rv = sqsh__buffer_append(&new_buffer, &data[inner_offset], size);
		if (rv < 0) {
			goto out;
		}
	}

	/*
	 * 2. The remainder of the requested range is read from the iterator
	 */
	sqsh_index_t inner_offset = 0;
	if (new_offset > iterator_offset) {
		inner_offset = new_offset - iterator_offset;
	}
	if (inner_offset >= impl->size(iterator)) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
	// We expect, that the iterator is already at the correct position.
	while (true) {
		const size_t size = SQSH_MIN(
				impl->size(iterator) - inner_offset,
				target_size - sqsh__buffer_size(&new_buffer));
		const uint8_t *data = impl->data(iterator);

		rv = sqsh__buffer_append(&new_buffer, &data[inner_offset], size);
		if (rv < 0) {
			goto out;
		}

		inner_offset = 0;
		if (sqsh__buffer_size(&new_buffer) < target_size) {
			const size_t desired_size =
					target_size - sqsh__buffer_size(&new_buffer);
			reader->iterator_offset += impl->size(iterator);
			rv = impl->next(iterator, desired_size);
			if (rv < 0) {
				goto out;
			} else if (rv == 0) {
				rv = -SQSH_ERROR_OUT_OF_BOUNDS;
				goto out;
			}
		} else {
			break;
		}
	}

	rv = sqsh__buffer_move(&reader->buffer, &new_buffer);
	if (rv < 0) {
		goto out;
	}
	reader->data_offset = new_offset;
	reader->buffer_offset = new_offset;
	reader->data = sqsh__buffer_data(&reader->buffer);
	reader->data_size = sqsh__buffer_size(&reader->buffer);

out:
	sqsh__buffer_cleanup(&new_buffer);
	return rv;
}

int
sqsh__reader_advance(
		struct SqshReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	sqsh_index_t new_offset;
	sqsh_index_t new_end_offset;
	if (SQSH_ADD_OVERFLOW(offset, reader->data_offset, &new_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(new_offset, size, &new_end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	// Finding the start of the new range.
	if (new_offset >= reader->iterator_offset) {
		rv = iterator_forward_to(reader, new_offset, 1);
		if (rv < 0) {
			return rv;
		}
		reader->data = reader->impl->data(reader->iterator);
		reader->data_size = reader->impl->size(reader->iterator);
		reader->data_offset = reader->iterator_offset;
		sqsh__buffer_cleanup(&reader->buffer);
	}

	// Forward the data pointer to the requested offset.
	const sqsh_index_t inner_offset = new_offset - reader->data_offset;
	reader->data += inner_offset;
	reader->data_size -= inner_offset;
	reader->data_offset = new_offset;

	reader->size = size;

	// If the requested range is not covered by the iterator,
	// turn it into a buffer and extend it.
	if (new_end_offset > reader->data_offset + reader->data_size) {
		rv = map_buffered(reader, new_offset, new_end_offset);
		if (rv < 0) {
			return rv;
		}
	}

	return 0;
}

const uint8_t *
sqsh__reader_data(const struct SqshReader *reader) {
	return reader->data;
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
