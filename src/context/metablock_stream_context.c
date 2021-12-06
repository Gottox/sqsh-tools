/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @file        : metablock_stream_context
 * @created     : Sunday Dec 05, 2021 10:13:34 CET
 */

#include "metablock_stream_context.h"
#include "../compression/buffer.h"
#include "../data/metablock.h"
#include "../error.h"
#include "metablock_context.h"
#include "superblock_context.h"
#include <stdint.h>

HSQS_NO_UNUSED int
hsqs_metablock_stream_init(
		struct HsqsMetablockStreamContext *context,
		const struct HsqsSuperblockContext *superblock, uint64_t address,
		uint64_t max_address) {
	int rv = 0;

	context->superblock = superblock;
	context->base_address = address;
	rv = hsqs_metablock_stream_seek(context, address, 0);
	if (rv < 0) {
		goto out;
	}
out:
	if (rv < 0) {
		hsqs_metablock_stream_cleanup(context);
	}
	return rv;
}

int
hsqs_metablock_stream_seek_ref(
		struct HsqsMetablockStreamContext *context, uint64_t ref) {
	uint64_t address_offset = ref >> 16;
	uint16_t index = ref & 0xFFFF;

	return hsqs_metablock_stream_seek(context, address_offset, index);
}

int
hsqs_metablock_stream_seek(
		struct HsqsMetablockStreamContext *context, uint64_t address_offset,
		uint32_t buffer_offset) {
	int rv = 0;
	hsqs_buffer_cleanup(&context->buffer);

	if (ADD_OVERFLOW(
				context->base_address, address_offset,
				&context->current_address)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	context->buffer_offset = buffer_offset;

	enum HsqsSuperblockCompressionId compression_id =
			hsqs_superblock_compression_id(context->superblock);
	rv = hsqs_buffer_init(
			&context->buffer, compression_id, HSQS_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

static int
add_block(struct HsqsMetablockStreamContext *context) {
	int rv = 0;
	struct HsqsMetablockContext metablock = {0};
	uint64_t address = context->current_address;
	uint32_t metablock_size;

	rv = hsqs_metablock_init(&metablock, context->superblock, address);
	if (rv < 0) {
		goto out;
	}
	metablock_size =
			HSQS_SIZEOF_METABLOCK + hsqs_metablock_compressed_size(&metablock);
	if (ADD_OVERFLOW(address, metablock_size, &context->current_address)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = hsqs_metablock_to_buffer(&metablock, &context->buffer);
	if (rv < 0) {
		goto out;
	}

out:
	hsqs_metablock_cleanup(&metablock);
	return rv;
}

HSQS_NO_UNUSED int
hsqs_metablock_stream_more(
		struct HsqsMetablockStreamContext *context, uint64_t size) {
	int rv = 0;
	while (hsqs_metablock_stream_size(context) < size) {
		rv = add_block(context);
		if (rv < 0) {
			return rv;
		}
	}
	return rv;
}

const uint8_t *
hsqs_metablock_stream_data(struct HsqsMetablockStreamContext *context) {
	if (hsqs_metablock_stream_size(context) > 0) {
		return &hsqs_buffer_data(&context->buffer)[context->buffer_offset];
	} else {
		return NULL;
	}
}

size_t
hsqs_metablock_stream_size(struct HsqsMetablockStreamContext *context) {
	size_t buffer_size = hsqs_buffer_size(&context->buffer);

	if (buffer_size > context->buffer_offset) {
		return buffer_size - context->buffer_offset;
	} else {
		return 0;
	}
}

int
hsqs_metablock_stream_cleanup(struct HsqsMetablockStreamContext *context) {
	hsqs_buffer_cleanup(&context->buffer);
	return 0;
}
