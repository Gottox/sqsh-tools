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

#include "trailing_context.h"
#include "../hsqs.h"
#include "../mapper/mapper.h"
#include "../utils.h"

int
hsqs_trailing_init(struct HsqsTrailingContext *context, struct Hsqs *hsqs) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	uint64_t trailing_start = hsqs_superblock_bytes_used(superblock);
	size_t archive_size = hsqs_mapper_size(&hsqs->mapper);
	uint64_t trailing_size;

	if (archive_size <= trailing_start) {
		return HSQS_ERROR_TODO;
	}

	if (SUB_OVERFLOW(archive_size, trailing_start, &trailing_size)) {
		return HSQS_ERROR_TODO;
	}

	return hsqs_request_map(
			hsqs, context->mapping, trailing_start, trailing_size);
}

size_t
hsqs_trailing_size(struct HsqsTrailingContext *context) {
	return hsqs_mapping_size(context->mapping);
}

const uint8_t *
hsqs_trailing_data(struct HsqsTrailingContext *context) {
	return hsqs_mapping_data(context->mapping);
}

int
hsqs_trailing_cleanup(struct HsqsTrailingContext *context) {
	return hsqs_mapping_unmap(context->mapping);
}
