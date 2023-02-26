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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : trailing_context.c
 */

#include "../../include/sqsh_context_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__trailing_init(
		struct SqshTrailingContext *context, struct SqshArchive *sqsh) {
	int rv = 0;
	const struct SqshSuperblockContext *superblock =
			sqsh_archive_superblock(sqsh);
	uint64_t trailing_start = sqsh_superblock_bytes_used(superblock);
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(sqsh);
	size_t archive_size = sqsh__map_manager_size(map_manager);
	uint64_t trailing_size;

	if (archive_size <= trailing_start) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}

	if (SQSH_SUB_OVERFLOW(archive_size, trailing_start, &trailing_size)) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}

	rv = sqsh__map_cursor_init(
			&context->cursor, map_manager, trailing_start, trailing_size);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__map_cursor_all(&context->cursor);
	if (rv < 0) {
		goto out;
	}
out:
	if (rv < 0) {
		sqsh__trailing_cleanup(context);
	}
	return rv;
}

size_t
sqsh_trailing_size(const struct SqshTrailingContext *context) {
	return sqsh__map_cursor_size(&context->cursor);
}

const uint8_t *
sqsh_trailing_data(const struct SqshTrailingContext *context) {
	return sqsh__map_cursor_data(&context->cursor);
}

int
sqsh__trailing_cleanup(struct SqshTrailingContext *context) {
	return sqsh__map_cursor_cleanup(&context->cursor);
}
