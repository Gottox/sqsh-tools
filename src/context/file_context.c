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
 * @author       Enno Boland (mail@eboland.de)
 * @file         file_context.c
 */

#include <sqsh.h>
#include <sqsh_compression.h>
#include <sqsh_context.h>
#include <sqsh_error.h>
#include <sqsh_table.h>
#include <stdint.h>
#include <stdio.h>

static uint64_t
datablock_offset(struct SqshFileContext *context, uint32_t block_index) {
	uint64_t offset = 0;

	for (uint32_t i = 0; i < block_index; i++) {
		if (ADD_OVERFLOW(
					offset, sqsh_inode_file_block_size(context->inode, i),
					&offset)) {
			return UINT64_MAX;
		}
	}
	return offset;
}

int
sqsh_file_init(
		struct SqshFileContext *context, struct SqshInodeContext *inode) {
	int rv = 0;
	struct Sqsh *sqsh = inode->sqsh;
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_FILE) {
		return -SQSH_ERROR_NOT_A_FILE;
	}

	context->inode = inode;
	context->block_size = sqsh_superblock_block_size(superblock);
	context->mapper = sqsh_mapper(sqsh);
	context->compression = sqsh_data_compression(sqsh);

	if (sqsh_inode_file_has_fragment(inode)) {
		rv = sqsh_fragment_table(sqsh, &context->fragment_table);
		if (rv < 0) {
			return rv;
		}
	} else {
		context->fragment_table = NULL;
	}

	rv = sqsh_buffer_init(&context->buffer);
	if (rv < 0) {
		return rv;
	}

	return sqsh_file_seek(context, 0);
}

int
sqsh_file_seek(struct SqshFileContext *context, uint64_t seek_pos) {
	if (seek_pos > sqsh_inode_file_size(context->inode)) {
		return -SQSH_ERROR_SEEK_OUT_OF_RANGE;
	}
	context->seek_pos = seek_pos;

	return 0;
}

int
sqsh_file_read(struct SqshFileContext *context, uint64_t size) {
	int rv = 0;
	struct SqshMapping mapping = {0};
	struct SqshFragmentTable *table = context->fragment_table;
	struct SqshBuffer *buffer = &context->buffer;
	uint64_t start_block = sqsh_inode_file_blocks_start(context->inode);
	bool is_compressed;
	uint32_t block_index = context->seek_pos / context->block_size;
	uint32_t block_count = sqsh_inode_file_block_count(context->inode);
	uint64_t block_offset = datablock_offset(context, block_index);
	uint64_t block_whole_size;

	uint32_t outer_block_size;
	uint64_t outer_offset = 0;

	if (SUB_OVERFLOW(
				datablock_offset(context, block_count), block_offset,
				&block_whole_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	// TODO: Handle integer overflow when adding start_block to block_offset
	rv = sqsh_mapper_map(
			&mapping, context->mapper, start_block + block_offset,
			block_whole_size);

	if (rv < 0) {
		goto out;
	}
	if (sqsh_inode_file_size(context->inode) > size) {
		rv = SQSH_ERROR_SIZE_MISSMATCH;
	}

	for (; block_index < block_count && sqsh_file_size(context) < size;
		 block_index++) {
		is_compressed = sqsh_inode_file_block_is_compressed(
				context->inode, block_index);
		outer_block_size =
				sqsh_inode_file_block_size(context->inode, block_index);

		const uint8_t *data = &sqsh_mapping_data(&mapping)[outer_offset];
		const size_t size = outer_block_size;
		if (is_compressed) {
			rv = sqsh_compression_decompress_to_buffer(
					context->compression, buffer, data, size);
		} else {
			rv = sqsh_buffer_append(buffer, data, size);
		}
		if (rv < 0) {
			goto out;
		}
		// TODO: Handle integer overflow when adding outer_offset to
		// outer_block_size.
		outer_offset += outer_block_size;
	}

	if (sqsh_file_size(context) < size) {
		if (!sqsh_inode_file_has_fragment(context->inode)) {
			rv = -SQSH_ERROR_TODO;
			goto out;
		}

		rv = sqsh_fragment_table_to_buffer(table, context->inode, buffer);
		if (rv < 0) {
			goto out;
		}

		if (sqsh_file_size(context) < size) {
			rv = -SQSH_ERROR_TODO;
			goto out;
		}
	}

out:
	sqsh_mapping_unmap(&mapping);
	return rv;
}

const uint8_t *
sqsh_file_data(struct SqshFileContext *context) {
	uint32_t block_size = context->block_size;
	off_t offset = context->seek_pos % block_size;

	if (sqsh_file_size(context) == 0) {
		return NULL;
	} else {
		return &sqsh_buffer_data(&context->buffer)[offset];
	}
}

uint64_t
sqsh_file_size(struct SqshFileContext *context) {
	uint32_t block_size = context->block_size;
	size_t offset = context->seek_pos % block_size;
	size_t buffer_size = sqsh_buffer_size(&context->buffer);

	if (buffer_size < offset)
		return 0;
	else
		return buffer_size - offset;
}

int
sqsh_file_cleanup(struct SqshFileContext *context) {
	sqsh_buffer_cleanup(&context->buffer);

	return 0;
}
