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
 * @file        : metablock_context
 * @created     : Saturday Dec 04, 2021 16:20:07 CET
 */

#include "metablock_context.h"
#include "../data/metablock.h"
#include "../error.h"
#include "../hsqs.h"
#include <stdint.h>

static const struct HsqsMetablock *
get_metablock(const struct HsqsMetablockContext *context) {
	return (const struct HsqsMetablock *)hsqs_mapping_data(&context->mapping);
}

static int
read_buffer(struct HsqsMetablockContext *context, struct HsqsBuffer *buffer) {
	int rv = 0;
	const struct HsqsMetablock *metablock = get_metablock(context);
	uint32_t size = hsqs_data_metablock_size(metablock);
	bool is_compressed = hsqs_data_metablock_is_compressed(metablock);
	uint32_t map_size;

	if (size > HSQS_METABLOCK_BLOCK_SIZE) {
		return -HSQS_ERROR_METABLOCK_TOO_BIG;
	}
	if (ADD_OVERFLOW(size, HSQS_SIZEOF_METABLOCK, &map_size)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = hsqs_mapping_resize(&context->mapping, map_size);
	if (rv < 0) {
		goto out;
	}

	// metablock may has moved after resize, so re-request it:
	metablock = get_metablock(context);
	const uint8_t *data = hsqs_data_metablock_data(metablock);

	rv = hsqs_buffer_append_block(buffer, data, size, is_compressed);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
hsqs_metablock_init(
		struct HsqsMetablockContext *context, struct Hsqs *hsqs,
		uint64_t address) {
	int rv = 0;

	rv = hsqs_request_map(
			hsqs, &context->mapping, address, HSQS_SIZEOF_METABLOCK);
	if (rv < 0) {
		goto out;
	}
	context->hsqs = hsqs;
	context->address = address;

out:
	if (rv < 0) {
		hsqs_metablock_cleanup(context);
	}

	return rv;
}

uint32_t
hsqs_metablock_compressed_size(const struct HsqsMetablockContext *context) {
	const struct HsqsMetablock *metablock = get_metablock(context);
	return hsqs_data_metablock_size(metablock);
}

static int
buffer_dtor(void *data) {
	struct HsqsBuffer *buffer = data;
	hsqs_buffer_cleanup(buffer);
	return 0;
}

int
hsqs_metablock_read(struct HsqsMetablockContext *context) {
	int rv = 0;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(context->hsqs);
	struct HsqsLruHashmap *cache = hsqs_metablock_cache(context->hsqs);

	if (context->buffer != NULL) {
		return 0;
	}

	context->ref = hsqs_lru_hashmap_get(cache, context->address);
	if (context->ref == NULL) {
		rv = hsqs_ref_count_new(
				&context->ref, sizeof(struct HsqsBuffer), buffer_dtor);
		if (rv < 0) {
			goto out;
		}
		if (rv < 0) {
			goto out;
		}
		context->buffer = hsqs_ref_count_retain(context->ref);
		rv = hsqs_buffer_init(
				context->buffer, hsqs_superblock_compression_id(superblock),
				HSQS_METABLOCK_BLOCK_SIZE);
		rv = hsqs_lru_hashmap_put(cache, context->address, context->ref);
		if (rv < 0) {
			goto out;
		}
		rv = read_buffer(context, context->buffer);
		if (rv < 0) {
			goto out;
		}
	} else {
		context->buffer = hsqs_ref_count_retain(context->ref);
	}

out:
	return rv;
}

int
hsqs_metablock_to_buffer(
		struct HsqsMetablockContext *context, struct HsqsBuffer *buffer) {
	int rv = 0;
	rv = hsqs_metablock_read(context);
	if (rv < 0) {
		return rv;
	}
	const struct HsqsBuffer *source_buffer = context->buffer;

	const uint8_t *source_data = hsqs_buffer_data(source_buffer);
	const uint64_t source_size = hsqs_buffer_size(source_buffer);

	return hsqs_buffer_append_block(buffer, source_data, source_size, false);
}

int
hsqs_metablock_cleanup(struct HsqsMetablockContext *context) {
	hsqs_ref_count_release(context->ref);
	hsqs_mapping_unmap(&context->mapping);
	return 0;
}
