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
 * @file        : datablock_context
 * @created     : Tuesday Sep 14, 2021 22:56:07 CEST
 */

#include "datablock_context.h"
#include "../error.h"
#include "inode_context.h"
#include "metablock_context.h"
#include "superblock_context.h"

#include <stdint.h>

int
hsqs_datablock_init(
		struct HsqsDatablockContext *context,
		const struct HsqsInodeContext *inode) {
	size_t block_size = hsqs_superblock_block_size(inode->extract.superblock);
	int rv = 0;

	if (hsqs_inode_type(inode) != HSQS_INODE_TYPE_FILE) {
		return -HSQS_ERROR_NOT_A_FILE;
	}
	uint64_t file_size = hsqs_inode_file_size(inode);
	context->superblock = inode->extract.superblock;
	context->inode = inode;
	if (hsqs_inode_file_fragment_block_index(inode) == HSQS_INODE_NO_FRAGMENT) {
		context->blocks_count = hsqs_divide_ceil_u32(file_size, block_size);
	} else {
		context->blocks_count = file_size / block_size;
	}
	if (context->blocks_count == 0) {
		return -HSQS_ERROR_NO_DATABLOCKS;
	}
	context->blocks = hsqs_superblock_data_from_offset(
			context->superblock, hsqs_inode_file_blocks_start(inode));
	if (context->blocks == NULL) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	rv = hsqs_buffer_init(&context->buffer, context->superblock, block_size);
	if (rv < 0) {
		return rv;
	}
	return rv;
}

static const uint64_t
datablock_offset(struct HsqsDatablockContext *context, uint64_t index) {
	off_t offset = 0;

	for (int i = 0; i < index; i++) {
		offset += hsqs_inode_file_block_size(context->inode, i);
	}

	return offset;
}

int
hsqs_datablock_seek(struct HsqsDatablockContext *context, uint64_t seek_pos) {
	size_t block_size = hsqs_superblock_block_size(context->superblock);
	uint64_t file_size = hsqs_inode_file_size(context->inode);

	if (seek_pos > file_size) {
		return -HSQS_ERROR_SEEK_OUT_OF_RANGE;
	}

	uint32_t datablock_index = seek_pos / block_size;
	if (datablock_index >= context->blocks_count) {
		return -HSQS_ERROR_SEEK_IN_FRAGMENT;
	}

	context->datablock_index = datablock_index;

	context->datablock_offset = seek_pos % block_size;

	return 0;
}

void *
hsqs_datablock_data(const struct HsqsDatablockContext *context) {
	return &context->buffer.data[context->datablock_offset];
}

size_t
hsqs_datablock_size(const struct HsqsDatablockContext *context) {
	const size_t buffer_size = hsqs_buffer_size(&context->buffer);
	if (context->datablock_offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - context->datablock_offset;
	}
}

int
hsqs_datablock_read(struct HsqsDatablockContext *context, uint64_t size) {
	int rv = 0;

	uint64_t compressed_offset =
			datablock_offset(context, context->datablock_index);

	while (hsqs_datablock_size(context) < size &&
		   context->datablock_index < context->blocks_count) {
		bool is_compressed = hsqs_inode_file_block_is_compressed(
				context->inode, context->datablock_index);
		uint64_t compressed_size = hsqs_inode_file_block_size(
				context->inode, context->datablock_index);
		rv = hsqs_buffer_append(
				&context->buffer, &context->blocks[compressed_offset],
				compressed_size, is_compressed);
		if (rv < 0) {
			goto out;
		}
		compressed_offset += compressed_size;
		context->datablock_index++;
	}

out:
	return rv;
}

int
hsqs_datablock_clean(struct HsqsDatablockContext *context) {
	return hsqs_buffer_cleanup(&context->buffer);
}
