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

#include "../../include/sqsh_mapper.h"

#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__map_cursor_init(
		struct SqshMapCursor *cursor, struct SqshMapper *mapper,
		const uint64_t start_address, const uint64_t upper_limit) {
	cursor->offset = 0;
	cursor->upper_limit = upper_limit;
	cursor->mapper = mapper;
	return sqsh_mapper_map(
			&cursor->mapping, mapper, start_address,
			SQSH_MIN(4096, upper_limit - start_address));
}

int
sqsh__map_cursor_advance(
		struct SqshMapCursor *cursor, sqsh_index_t offset, size_t size) {
	size_t new_size;

	if (SQSH_ADD_OVERFLOW(cursor->offset, offset, &cursor->offset)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(cursor->offset, size, &new_size)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (new_size > cursor->upper_limit) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	return sqsh_mapping_resize(&cursor->mapping, new_size);
}

int
sqsh__map_cursor_all(struct SqshMapCursor *cursor) {
	return sqsh__map_cursor_advance(cursor, 0, cursor->upper_limit);
}

const uint8_t *
sqsh__map_cursor_data(const struct SqshMapCursor *cursor) {
	return &sqsh_mapping_data(&cursor->mapping)[cursor->offset];
}

size_t
sqsh__map_cursor_size(const struct SqshMapCursor *cursor) {
	return sqsh_mapping_size(&cursor->mapping) - cursor->offset;
}

int
sqsh__map_cursor_cleanup(struct SqshMapCursor *cursor) {
	sqsh_mapping_unmap(&cursor->mapping);
	return 0;
}
