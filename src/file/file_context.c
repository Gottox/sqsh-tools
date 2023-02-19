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
 * @file         file_context.c
 */

#include "../../include/sqsh.h"
#include "../../include/sqsh_compression_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_inode_private.h"
#include "../../include/sqsh_table.h"
#include "../utils.h"
#include "sqsh_common.h"

static uint64_t
datablock_offset(struct SqshFileContext *context, uint32_t block_index) {
	uint64_t offset = 0;

	for (uint32_t i = 0; i < block_index; i++) {
		if (SQSH_ADD_OVERFLOW(
					offset, sqsh_inode_file_block_size(context->inode, i),
					&offset)) {
			return UINT64_MAX;
		}
	}
	return offset;
}

int
sqsh__file_init(
		struct SqshFileContext *context, const struct SqshInodeContext *inode) {
	int rv = 0;
	struct Sqsh *sqsh = inode->sqsh;
	const struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_FILE) {
		return -SQSH_ERROR_NOT_A_FILE;
	}

	context->inode = inode;
	context->block_size = sqsh_superblock_block_size(superblock);
	context->mapper = sqsh_mapper(sqsh);
	context->compression = sqsh_compression_data(sqsh);

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

struct SqshFileContext *
sqsh_file_new(const struct SqshInodeContext *inode, int *err) {
	struct SqshFileContext *context = calloc(1, sizeof(struct SqshFileContext));
	if (context == NULL) {
		return NULL;
	}
	*err = sqsh__file_init(context, inode);
	if (*err < 0) {
		free(context);
		return NULL;
	}
	return context;
}

int
sqsh_file_seek(struct SqshFileContext *context, uint64_t seek_pos) {
	if (seek_pos > sqsh_inode_file_size(context->inode)) {
		return -SQSH_ERROR_SEEK_OUT_OF_RANGE;
	}
	context->seek_pos = seek_pos;

	return 0;
}

static int
attach_fragment(struct SqshFileContext *context, const uint64_t size) {
	const struct SqshFragmentTable *table = context->fragment_table;
	const struct SqshInodeContext *inode = context->inode;
	struct SqshBuffer *buffer = &context->buffer;
	int rv = 0;

	if (sqsh_file_size(context) >= size) {
		return 0;
	}

	if (!sqsh_inode_file_has_fragment(inode)) {
		return -SQSH_ERROR_TODO;
	}

	rv = sqsh_fragment_table_to_buffer(table, inode, buffer);
	if (rv < 0) {
		return rv;
	}

	if (sqsh_file_size(context) < size) {
		return -SQSH_ERROR_TODO;
	}

	return 0;
}

int
sqsh_file_read(struct SqshFileContext *context, const uint64_t size) {
	int rv = 0;
	const uint64_t start_block = sqsh_inode_file_blocks_start(context->inode);
	const uint32_t block_count = sqsh_inode_file_block_count(context->inode);

	struct SqshMapCursor cursor = {0};
	struct SqshBuffer *buffer = &context->buffer;
	uint32_t block_index = context->seek_pos / context->block_size;
	uint64_t block_offset = datablock_offset(context, block_index);

	// TODO: check for a sane upper limit.
	rv = sqsh__map_cursor_init(
			&cursor, context->mapper, start_block, UINT64_MAX);
	if (rv < 0) {
		goto out;
	}

	for (; block_index < block_count && sqsh_file_size(context) < size;
		 block_index++) {
		const uint32_t block_size =
				sqsh_inode_file_block_size(context->inode, block_index);
		const bool is_compressed = sqsh_inode_file_block_is_compressed(
				context->inode, block_index);
		rv = sqsh__map_cursor_advance(&cursor, block_offset, block_size);
		if (rv < 0) {
			goto out;
		}

		const uint8_t *data = sqsh__map_cursor_data(&cursor);
		if (is_compressed) {
			rv = sqsh__compression_decompress_to_buffer(
					context->compression, buffer, data, block_size);
		} else {
			rv = sqsh_buffer_append(buffer, data, block_size);
		}
		if (rv < 0) {
			goto out;
		}
		block_offset = block_size;
	}

	rv = attach_fragment(context, size);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__map_cursor_cleanup(&cursor);
	return rv;
}

const uint8_t *
sqsh_file_data(struct SqshFileContext *context) {
	uint32_t block_size = context->block_size;
	sqsh_index_t offset = context->seek_pos % block_size;

	if (sqsh_file_size(context) == 0) {
		return NULL;
	} else {
		return &sqsh_buffer_data(&context->buffer)[offset];
	}
}

uint64_t
sqsh_file_size(const struct SqshFileContext *context) {
	const uint32_t block_size = context->block_size;
	const size_t offset = context->seek_pos % block_size;
	const size_t buffer_size = sqsh_buffer_size(&context->buffer);

	if (buffer_size < offset) {
		return 0;
	} else {
		return buffer_size - offset;
	}
}

int
sqsh__file_cleanup(struct SqshFileContext *context) {
	sqsh_buffer_cleanup(&context->buffer);

	return 0;
}

int
sqsh_file_free(struct SqshFileContext *context) {
	if (context == NULL) {
		return 0;
	}
	int rv = sqsh__file_cleanup(context);
	free(context);
	return rv;
}
