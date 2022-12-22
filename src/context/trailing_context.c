/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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

#include "../utils.h"
#include <sqsh.h>
#include <sqsh_context.h>

int
sqsh_trailing_init(struct SqshTrailingContext *context, struct Sqsh *sqsh) {
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);
	uint64_t trailing_start = sqsh_superblock_bytes_used(superblock);
	struct SqshMapper *mapper = sqsh_mapper(sqsh);
	size_t archive_size = sqsh_mapper_size(mapper);
	uint64_t trailing_size;

	if (archive_size <= trailing_start) {
		return -SQSH_ERROR_TODO;
	}

	if (SQSH_SUB_OVERFLOW(archive_size, trailing_start, &trailing_size)) {
		return -SQSH_ERROR_TODO;
	}

	return sqsh_mapper_map(
			context->mapping, mapper, trailing_start, trailing_size);
}

size_t
sqsh_trailing_size(struct SqshTrailingContext *context) {
	return sqsh_mapping_size(context->mapping);
}

const uint8_t *
sqsh_trailing_data(struct SqshTrailingContext *context) {
	return sqsh_mapping_data(context->mapping);
}

int
sqsh_trailing_cleanup(struct SqshTrailingContext *context) {
	return sqsh_mapping_unmap(context->mapping);
}
