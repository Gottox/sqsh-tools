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
 * @file         metablock_context.c
 */

#include "../../include/sqsh.h"
#include "../../include/sqsh_compression_private.h"
#include "../../include/sqsh_context_private.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"
#include <stdint.h>

static const struct SqshDataMetablock *
get_metablock(const struct SqshMetablockContext *context) {
	return (const struct SqshDataMetablock *)sqsh_mapping_data(
			&context->mapping);
}

int
sqsh__metablock_init(
		struct SqshMetablockContext *context, struct Sqsh *sqsh,
		uint64_t address) {
	int rv = 0;
	struct SqshMapper *mapper = sqsh_mapper(sqsh);

	rv = sqsh_mapper_map(
			&context->mapping, mapper, address, SQSH_SIZEOF_METABLOCK);
	if (rv < 0) {
		goto out;
	}
	context->compression = sqsh_metablock_compression(sqsh);

out:
	if (rv < 0) {
		sqsh__metablock_cleanup(context);
	}

	return rv;
}

uint32_t
sqsh__metablock_compressed_size(const struct SqshMetablockContext *context) {
	const struct SqshDataMetablock *metablock = get_metablock(context);
	return sqsh_data_metablock_size(metablock);
}

int
sqsh__metablock_to_buffer(
		struct SqshMetablockContext *context, struct SqshBuffer *buffer) {
	int rv = 0;
	const struct SqshDataMetablock *metablock = get_metablock(context);
	uint32_t size = sqsh_data_metablock_size(metablock);
	bool is_compressed = sqsh_data_metablock_is_compressed(metablock);
	uint32_t map_size;

	if (size > SQSH_METABLOCK_BLOCK_SIZE) {
		return -SQSH_ERROR_METABLOCK_TOO_BIG;
	}
	if (SQSH_ADD_OVERFLOW(size, SQSH_SIZEOF_METABLOCK, &map_size)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = sqsh_mapping_resize(&context->mapping, map_size);
	if (rv < 0) {
		goto out;
	}

	// metablock may has moved after resize, so re-request it:
	metablock = get_metablock(context);
	const uint8_t *data = sqsh_data_metablock_data(metablock);

	if (is_compressed) {
		rv = sqsh__compression_decompress_to_buffer(
				context->compression, buffer, data, size);
	} else {
		rv = sqsh_buffer_append(buffer, data, size);
	}
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh__metablock_cleanup(struct SqshMetablockContext *context) {
	sqsh_mapping_unmap(&context->mapping);
	return 0;
}
