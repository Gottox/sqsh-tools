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
 * @created     : Saturday Sep 04, 2021 23:15:47 CEST
 */

#include "metablock_context.h"
#include "../compression/buffer.h"
#include "../data/metablock.h"
#include "../error.h"
#include "superblock_context.h"
#include <stdint.h>

#define BLOCK_SIZE 8192

static int
metablock_bounds_check(
		const struct HsqsSuperblockContext *superblock,
		const struct HsqsMetablock *block) {
	uint64_t offset;
	uint64_t offset_header_end;
	uint64_t data_bounds;
	uint64_t bytes_used = hsqs_superblock_bytes_used(superblock);

	if (SUB_OVERFLOW(
				(uint64_t)block, (uint64_t)superblock->superblock, &offset)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	if (ADD_OVERFLOW(offset, HSQS_SIZEOF_METABLOCK, &offset_header_end)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	if (offset_header_end > bytes_used) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	const size_t data_size = hsqs_data_metablock_size(block);
	if (ADD_OVERFLOW((uint64_t)offset_header_end, data_size, &data_bounds)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	if (data_bounds > bytes_used) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	return 0;
}

int
hsqs_metablock_from_offset(
		const struct HsqsMetablock **metablock,
		const struct HsqsSuperblockContext *superblock, off_t offset) {
	int rv = 0;
	const struct HsqsMetablock *block =
			hsqs_superblock_data_from_offset(superblock, offset);
	if (block == NULL) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	rv = metablock_bounds_check(superblock, block);
	if (rv < 0) {
		return rv;
	}
	*metablock = block;
	return rv;
}

static const struct HsqsMetablock *
hsqs_metablock_from_start_block(
		const struct HsqsSuperblockContext *superblock,
		const struct HsqsMetablock *start_block, off_t offset) {
	const uint8_t *tmp = (uint8_t *)start_block;
	const struct HsqsMetablock *block =
			(const struct HsqsMetablock *)&tmp[offset];

	if (metablock_bounds_check(superblock, block) < 0) {
		return NULL;
	} else {
		return block;
	}
}

int
hsqs_metablock_init(
		struct HsqsMetablockContext *extract,
		const struct HsqsSuperblockContext *superblock, off_t start_block) {
	extract->start_block = start_block;
	extract->index = 0;
	extract->offset = 0;
	extract->superblock = superblock;
	return hsqs_metablock_seek(extract, 0, 0);
}

int
hsqs_metablock_seek(
		struct HsqsMetablockContext *metablock, const off_t index,
		const off_t offset) {
	int rv = 0;

	hsqs_buffer_cleanup(&metablock->buffer);

	metablock->index = index;
	metablock->offset = offset;
	hsqs_buffer_cleanup(&metablock->buffer);
	rv = hsqs_buffer_init(
			&metablock->buffer, metablock->superblock, BLOCK_SIZE);

	return rv;
}

int
hsqs_metablock_more(struct HsqsMetablockContext *extract, const size_t size) {
	int rv = 0;
	const struct HsqsMetablock *start_block = NULL;

	rv = hsqs_metablock_from_offset(
			&start_block, extract->superblock, extract->start_block);

	if (rv < 0) {
		return rv;
	}

	for (; rv == 0 && size > hsqs_metablock_size(extract);) {
		const struct HsqsMetablock *block = hsqs_metablock_from_start_block(
				extract->superblock, start_block, extract->index);
		if (block == NULL) {
			return -HSQS_ERROR_TODO;
		}
		const size_t metablock_size = hsqs_data_metablock_size(block);
		if (metablock_size == 0) {
			return -HSQS_ERROR_METABLOCK_ZERO_SIZE;
		}

		const void *block_data = hsqs_data_metablock_data(block);
		bool is_compressed = hsqs_data_metablock_is_compressed(block);
		rv = hsqs_buffer_append(
				&extract->buffer, block_data, metablock_size, is_compressed);
		extract->index += HSQS_SIZEOF_METABLOCK + metablock_size;
	}
	return rv;
}

const uint8_t *
hsqs_metablock_data(const struct HsqsMetablockContext *extract) {
	return &extract->buffer.data[extract->offset];
}

size_t
hsqs_metablock_size(const struct HsqsMetablockContext *extract) {
	const size_t buffer_size = hsqs_buffer_size(&extract->buffer);
	if (extract->offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - extract->offset;
	}
}

int
hsqs_metablock_cleanup(struct HsqsMetablockContext *extract) {
	return hsqs_buffer_cleanup(&extract->buffer);
}
