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
 * @file        : content_context
 * @created     : Thursday Oct 07, 2021 09:27:20 CEST
 */

#include "content_context.h"
#include "../buffer/buffer.h"
#include "../error.h"
#include "../hsqs.h"
#include "../mapper/mapper.h"
#include "inode_context.h"
#include "superblock_context.h"
#include <stdint.h>
#include <stdio.h>

static uint64_t
datablock_offset(struct HsqsFileContext *context, uint32_t block_index) {
	uint64_t offset = 0;

	for (uint32_t i = 0; i < block_index; i++) {
		offset += hsqs_inode_file_block_size(context->inode, i);
	}
	return offset;
}

int
hsqs_content_init(
		struct HsqsFileContext *context, struct HsqsInodeContext *inode) {
	int rv = 0;
	struct Hsqs *hsqs = inode->hsqs;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);

	if (hsqs_inode_type(inode) != HSQS_INODE_TYPE_FILE) {
		return -HSQS_ERROR_NOT_A_FILE;
	}

	context->inode = inode;
	context->block_size = hsqs_superblock_block_size(superblock);
	context->hsqs = hsqs;

	if (hsqs_inode_file_has_fragment(inode)) {
		rv = hsqs_fragment_table(context->hsqs, &context->fragment_table);
		if (rv < 0) {
			return rv;
		}
	} else {
		context->fragment_table = NULL;
	}

	enum HsqsSuperblockCompressionId compression_id =
			hsqs_superblock_compression_id(superblock);
	rv = hsqs_buffer_init(
			&context->buffer, compression_id, context->block_size);
	if (rv < 0) {
		return rv;
	}

	return hsqs_content_seek(context, 0);
}

int
hsqs_content_seek(struct HsqsFileContext *context, uint64_t seek_pos) {
	if (seek_pos > hsqs_inode_file_size(context->inode)) {
		return -HSQS_ERROR_SEEK_OUT_OF_RANGE;
	}
	context->seek_pos = seek_pos;

	return 0;
}

int
hsqs_content_read(struct HsqsFileContext *context, uint64_t size) {
	int rv = 0;
	struct HsqsMapping mapping = {0};
	struct HsqsFragmentTable *table = context->fragment_table;
	struct HsqsBuffer *buffer = &context->buffer;
	uint64_t start_block = hsqs_inode_file_blocks_start(context->inode);
	bool is_compressed;
	uint32_t block_index = context->seek_pos / context->block_size;
	uint32_t block_count = hsqs_inode_file_block_count(context->inode);
	uint64_t block_offset = datablock_offset(context, block_index);
	uint64_t block_whole_size =
			datablock_offset(context, block_count) - block_offset;
	uint32_t outer_block_size;
	uint64_t outer_offset = 0;

	rv = hsqs_request_map(
			context->hsqs, &mapping, start_block + block_offset,
			block_whole_size);

	if (rv < 0) {
		goto out;
	}
	if (hsqs_inode_file_size(context->inode) > size) {
		rv = HSQS_ERROR_SIZE_MISSMATCH;
	}

	for (; block_index < block_count && hsqs_content_size(context) < size;
		 block_index++) {
		is_compressed = hsqs_inode_file_block_is_compressed(
				context->inode, block_index);
		outer_block_size =
				hsqs_inode_file_block_size(context->inode, block_index);

		rv = hsqs_buffer_append_block(
				buffer, &hsqs_mapping_data(&mapping)[outer_offset],
				outer_block_size, is_compressed);
		if (rv < 0) {
			goto out;
		}
		outer_offset += outer_block_size;
	}

	if (hsqs_content_size(context) < size) {
		if (!hsqs_inode_file_has_fragment(context->inode)) {
			rv = -HSQS_ERROR_TODO;
			goto out;
		}

		rv = hsqs_fragment_table_to_buffer(table, context->inode, buffer);
		if (rv < 0) {
			goto out;
		}

		if (hsqs_content_size(context) < size) {
			rv = -HSQS_ERROR_TODO;
			goto out;
		}
	}

out:
	hsqs_mapping_unmap(&mapping);
	return rv;
}

const uint8_t *
hsqs_content_data(struct HsqsFileContext *context) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(context->hsqs);
	uint32_t block_size = hsqs_superblock_block_size(superblock);
	off_t offset = context->seek_pos % block_size;

	if (hsqs_content_size(context) == 0) {
		return NULL;
	} else {
		return &hsqs_buffer_data(&context->buffer)[offset];
	}
}

uint64_t
hsqs_content_size(struct HsqsFileContext *context) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(context->hsqs);
	uint32_t block_size = hsqs_superblock_block_size(superblock);
	size_t offset = context->seek_pos % block_size;
	size_t buffer_size = hsqs_buffer_size(&context->buffer);

	if (buffer_size < offset)
		return 0;
	else
		return buffer_size - offset;
}

int
hsqs_content_cleanup(struct HsqsFileContext *context) {
	hsqs_buffer_cleanup(&context->buffer);

	return 0;
}
