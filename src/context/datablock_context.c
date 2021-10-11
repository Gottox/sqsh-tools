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
squash_datablock_init(struct SquashDatablockContext *context,
		const struct SquashInodeContext *inode) {
	size_t block_size = squash_superblock_block_size(inode->extract.superblock);
	int rv = 0;

	if (squash_inode_type(inode) != SQUASH_INODE_TYPE_FILE) {
		return -SQUASH_ERROR_NOT_A_FILE;
	}
	uint64_t file_size = squash_inode_file_size(inode);
	context->superblock = inode->extract.superblock;
	context->inode = inode;
	context->blocks = (const uint8_t *)context->superblock +
			squash_inode_file_blocks_start(inode);
	if (squash_inode_file_fragment_block_index(inode) ==
			SQUASH_INODE_NO_FRAGMENT) {
		context->blocks_count = squash_divide_ceil_u32(file_size, block_size);
	} else {
		context->blocks_count = file_size / block_size;
	}

	rv = squash_buffer_init(&context->buffer, context->superblock, block_size);
	if (rv < 0) {
		return rv;
	}
	return rv;
}

static uint32_t
datablock_size(struct SquashDatablockContext *context, uint64_t index) {
	const struct SquashInodeContext *inode = context->inode;

	return squash_inode_file_block_size(inode, index) & ~(1 << 24);
}

static bool
datablock_is_compressed(
		struct SquashDatablockContext *context, uint64_t index) {
	const struct SquashInodeContext *inode = context->inode;

	return 0 == (squash_inode_file_block_size(inode, index) & (1 << 24));
}

static const uint64_t
datablock_offset(struct SquashDatablockContext *context, uint64_t index) {
	off_t offset = 0;

	for (int i = 0; i < index; i++) {
		offset += datablock_size(context, i);
	}

	return offset;
}

int
squash_datablock_seek(
		struct SquashDatablockContext *context, uint64_t seek_pos) {
	size_t block_size = squash_superblock_block_size(context->superblock);
	uint64_t file_size = squash_inode_file_size(context->inode);

	if (seek_pos > file_size) {
		return -SQUASH_ERROR_SEEK_OUT_OF_RANGE;
	}

	uint32_t datablock_index = seek_pos / block_size;
	if (datablock_index >= context->blocks_count) {
		return -SQUASH_ERROR_SEEK_IN_FRAGMENT;
	}

	context->datablock_index = datablock_index;

	context->datablock_offset = seek_pos % block_size;

	return 0;
}

void *
squash_datablock_data(const struct SquashDatablockContext *context) {
	return &context->buffer.data[context->datablock_offset];
}

size_t
squash_datablock_size(const struct SquashDatablockContext *context) {
	const size_t buffer_size = squash_buffer_size(&context->buffer);
	if (context->datablock_offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - context->datablock_offset;
	}
}

int
squash_datablock_read(struct SquashDatablockContext *context, uint64_t size) {
	int rv = 0;

	uint64_t compressed_offset =
			datablock_offset(context, context->datablock_index);

	while (squash_datablock_size(context) < size &&
			context->datablock_index < context->blocks_count) {
		bool is_compressed =
				datablock_is_compressed(context, context->datablock_index);
		uint64_t compressed_size =
				datablock_size(context, context->datablock_index);
		rv = squash_buffer_append(&context->buffer,
				&context->blocks[compressed_offset], compressed_size,
				is_compressed);
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
squash_datablock_clean(struct SquashDatablockContext *context) {
	return squash_buffer_cleanup(&context->buffer);
}
