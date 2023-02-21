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

#include "../../include/sqsh_metablock_private.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__metablock_cursor_init(
		struct SqshMetablockCursor *cursor, struct Sqsh *sqsh,
		const uint64_t start_address, const uint64_t upper_limit) {
	int rv;
	rv = sqsh__metablock_iterator_init(
			&cursor->iterator, sqsh, start_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__buffer_init(&cursor->buffer);
	if (rv < 0) {
		goto out;
	}
	cursor->size = 0;
	cursor->offset = 0;

out:
	if (rv < 0) {
		sqsh__metablock_cursor_cleanup(cursor);
	}
	return rv;
}

int
sqsh__metablock_cursor_advance(
		struct SqshMetablockCursor *cursor, sqsh_index_t offset, size_t size) {
	int rv;
	sqsh_index_t new_offset;
	sqsh_index_t end_offset;
	size_t new_size;

	if (SQSH_ADD_OVERFLOW(offset, cursor->offset, &new_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(new_offset, size, &new_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(new_offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	while (sqsh__buffer_size(&cursor->buffer) < end_offset) {
		rv = sqsh__metablock_iterator_next(&cursor->iterator);
		if (rv < 0) {
			return rv;
		}

		rv = sqsh__metablock_iterator_append_to_buffer(
				&cursor->iterator, &cursor->buffer);
		if (rv < 0) {
			return rv;
		}
	}
	cursor->offset = new_offset;
	cursor->size = size;
	return 0;
}

const uint8_t *
sqsh__metablock_cursor_data(const struct SqshMetablockCursor *cursor) {
	const uint8_t *data = sqsh__buffer_data(&cursor->buffer);

	return &data[cursor->offset];
}

size_t
sqsh__metablock_cursor_size(const struct SqshMetablockCursor *cursor) {
	return cursor->size;
}

int
sqsh__metablock_cursor_cleanup(struct SqshMetablockCursor *cursor) {
	sqsh__metablock_iterator_cleanup(&cursor->iterator);
	sqsh__buffer_cleanup(&cursor->buffer);

	return 0;
}
