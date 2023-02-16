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

#include "../../include/sqsh.h"
#include "../../include/sqsh_compression_private.h"
#include "../../include/sqsh_context.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_iterator_private.h"
#include "../utils.h"
#include "sqsh_compression_private.h"
#include "sqsh_context_private.h"
#include "sqsh_data_private.h"
#include "sqsh_mapper.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int
sqsh__metablock_iterator_init(
		struct SqshMetablockIterator *iterator, struct Sqsh *sqsh,
		uint64_t start_address, uint64_t upper_limit) {
	struct SqshMapper *mapper = sqsh_mapper(sqsh);
	struct SqshCompression *compression = sqsh_metablock_compression(sqsh);

	iterator->size = 0;
	iterator->is_compressed = false;
	iterator->compression = compression;
	return sqsh__map_cursor_init(
			&iterator->cursor, mapper, start_address, upper_limit);
}

int
sqsh__metablock_iterator_next(struct SqshMetablockIterator *iterator) {
	int rv = 0;

	rv = sqsh__map_cursor_advance(
			&iterator->cursor, iterator->size, SQSH_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	const struct SqshDataMetablock *metablock =
			(struct SqshDataMetablock *)sqsh__map_cursor_data(
					&iterator->cursor);
	iterator->size = sqsh_data_metablock_size(metablock);
	iterator->is_compressed = sqsh_data_metablock_is_compressed(metablock);

	rv = sqsh__map_cursor_advance(
			&iterator->cursor, SQSH_METABLOCK_BLOCK_SIZE, iterator->size);

out:
	return rv;
}

const uint8_t *
sqsh__metablock_iterator_data(const struct SqshMetablockIterator *iterator) {
	return sqsh__map_cursor_data(&iterator->cursor);
}

bool
sqsh__metablock_iterator_is_compressed(
		const struct SqshMetablockIterator *iterator) {
	return iterator->is_compressed;
}

size_t
sqsh__metablock_iterator_size(const struct SqshMetablockIterator *iterator) {
	return iterator->size;
}

int
sqsh__metablock_iterator_append_to_buffer(
		const struct SqshMetablockIterator *iterator,
		struct SqshBuffer *buffer) {
	const uint8_t *data = sqsh__metablock_iterator_data(iterator);
	size_t size = sqsh__metablock_iterator_size(iterator);
	bool is_compressed = sqsh__metablock_iterator_is_compressed(iterator);

	if (is_compressed) {
		return sqsh__compression_decompress_to_buffer(
				iterator->compression, buffer, data, size);
	} else {
		return sqsh_buffer_append(buffer, data, size);
	}
}

int
sqsh__metablock_iterator_cleanup(struct SqshMetablockIterator *iterator) {
	return sqsh__map_cursor_cleanup(&iterator->cursor);
}
