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
 * @file         metablock_stream_context.c
 */

#include "../../include/sqsh_context_private.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

SQSH_NO_UNUSED int
sqsh__metablock_stream_init(
		struct SqshMetablockStreamContext *context, struct Sqsh *sqsh,
		uint64_t address, uint64_t max_address) {
	// TODO check for max_address
	(void)max_address;
	int rv = 0;

	context->sqsh = sqsh;
	context->base_address = address;
	rv = sqsh__metablock_stream_seek(context, address, 0);
	if (rv < 0) {
		goto out;
	}
out:
	if (rv < 0) {
		sqsh__metablock_stream_cleanup(context);
	}
	return rv;
}

int
sqsh__metablock_stream_seek_ref(
		struct SqshMetablockStreamContext *context, uint64_t ref) {
	uint64_t address_offset = sqsh_address_ref_outer_offset(ref);
	uint16_t index = sqsh_address_ref_inner_offset(ref);

	return sqsh__metablock_stream_seek(context, address_offset, index);
}

int
sqsh__metablock_stream_seek(
		struct SqshMetablockStreamContext *context, uint64_t address_offset,
		uint32_t buffer_offset) {
	sqsh_buffer_drain(&context->buffer);

	if (SQSH_ADD_OVERFLOW(
				context->base_address, address_offset,
				&context->current_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	context->buffer_offset = buffer_offset;
	return 0;
}

static int
add_block(struct SqshMetablockStreamContext *context) {
	int rv = 0;
	struct SqshMetablockContext metablock = {0};
	uint64_t address = context->current_address;
	uint32_t metablock_size;

	rv = sqsh__metablock_init(&metablock, context->sqsh, address);
	if (rv < 0) {
		goto out;
	}
	metablock_size =
			SQSH_SIZEOF_METABLOCK + sqsh__metablock_compressed_size(&metablock);
	if (SQSH_ADD_OVERFLOW(address, metablock_size, &context->current_address)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = sqsh__metablock_to_buffer(&metablock, &context->buffer);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__metablock_cleanup(&metablock);
	return rv;
}

int
sqsh__metablock_stream_more(
		struct SqshMetablockStreamContext *context, uint64_t size) {
	int rv = 0;
	while (sqsh__metablock_stream_size(context) < size) {
		rv = add_block(context);
		if (rv < 0) {
			return rv;
		}
	}
	return rv;
}

const uint8_t *
sqsh__metablock_stream_data(const struct SqshMetablockStreamContext *context) {
	if (sqsh__metablock_stream_size(context) > 0) {
		return &sqsh_buffer_data(&context->buffer)[context->buffer_offset];
	} else {
		return NULL;
	}
}

size_t
sqsh__metablock_stream_size(const struct SqshMetablockStreamContext *context) {
	size_t buffer_size = sqsh_buffer_size(&context->buffer);

	if (buffer_size > context->buffer_offset) {
		return buffer_size - context->buffer_offset;
	} else {
		return 0;
	}
}

int
sqsh__metablock_stream_cleanup(struct SqshMetablockStreamContext *context) {
	sqsh_buffer_cleanup(&context->buffer);
	return 0;
}
