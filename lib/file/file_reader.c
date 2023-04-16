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
 * @file         file_reader.c
 */

#include "../../include/sqsh_file_private.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_inode_private.h"
#include "../utils/utils.h"

int
sqsh__file_reader_init(
		struct SqshFileReader *reader, const struct SqshInode *inode) {
	int rv = 0;
	reader->current_offset = 0;
	reader->current_size = 0;

	rv = sqsh__buffer_init(&reader->buffer);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_iterator_init(&reader->iterator, inode);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

struct SqshFileReader *
sqsh_file_reader_new(const struct SqshInode *inode, int *err) {
	struct SqshFileReader *context = calloc(1, sizeof(struct SqshFileReader));
	if (context == NULL) {
		return NULL;
	}
	*err = sqsh__file_reader_init(context, inode);
	if (*err < 0) {
		free(context);
		return NULL;
	}
	return context;
}

static int
file_reader_direct(struct SqshFileReader *reader, size_t size) {
	struct SqshFileIterator *iterator = &reader->iterator;
	const struct SqshInode *inode = iterator->inode;
	const struct SqshArchive *archive = inode->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t block_size = sqsh_superblock_block_size(superblock);

	uint32_t block_offset = reader->current_offset % block_size;
	reader->data = sqsh_file_iterator_data(iterator) + block_offset;
	reader->current_size = size;

	return 0;
}

static int
file_reader_buffered(struct SqshFileReader *reader, size_t size) {
	struct SqshFileIterator *iterator = &reader->iterator;
	const struct SqshInode *inode = iterator->inode;
	const struct SqshArchive *archive = inode->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t block_size = sqsh_superblock_block_size(superblock);
	const uint32_t block_offset = reader->current_offset % block_size;
	uint64_t end_offset;
	if (SQSH_ADD_OVERFLOW(reader->current_offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	int rv = 0;
	struct SqshBuffer *buffer = &reader->buffer;

	sqsh__buffer_drain(&reader->buffer);
	rv = sqsh__buffer_append(
			buffer, sqsh_file_iterator_data(iterator) + block_offset,
			sqsh_file_iterator_size(iterator) - block_offset);
	if (rv < 0) {
		goto out;
	}
	for (; (rv = sqsh_file_iterator_next(iterator, end_offset)) > 0;) {
		const uint8_t *data = sqsh_file_iterator_data(iterator);
		const size_t data_size = sqsh_file_iterator_size(iterator);
		rv = sqsh__buffer_append(buffer, data, data_size);
		if (rv < 0) {
			goto out;
		}
		if (sqsh__buffer_size(buffer) >= size) {
			break;
		};
	}
	if (rv < 0) {
		goto out;
	}
	if (sqsh__buffer_size(buffer) < size) {
		// Premature end of file
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	if (rv < 0) {
		goto out;
	}
	rv = 0;
	reader->current_offset = 0;
	reader->data = sqsh__buffer_data(buffer);
	reader->current_size = size;
out:
	return rv;
}

int
sqsh_file_reader_advance(
		struct SqshFileReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	struct SqshFileIterator *iterator = &reader->iterator;
	const struct SqshInode *inode = iterator->inode;
	const struct SqshArchive *archive = inode->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint32_t block_size = sqsh_superblock_block_size(superblock);

	uint64_t current_offset = reader->current_offset;
	if (SQSH_ADD_OVERFLOW(offset, current_offset, &current_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	uint64_t end_offset;
	if (SQSH_ADD_OVERFLOW(current_offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	sqsh_index_t skip_blocks = current_offset / block_size;

	rv = sqsh_file_iterator_skip(iterator, skip_blocks);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_file_iterator_next(iterator, end_offset);
	if (rv < 0) {
		goto out;
	}

	reader->current_offset = current_offset;

	if (sqsh_file_iterator_size(iterator) >= end_offset) {
		rv = file_reader_direct(reader, size);
	} else {
		rv = file_reader_buffered(reader, size);
	}

out:
	return rv;
}

const uint8_t *
sqsh_file_reader_data(struct SqshFileReader *reader) {
	return reader->data;
}

size_t
sqsh_file_reader_size(struct SqshFileReader *reader) {
	return reader->current_size;
}

int
sqsh__file_reader_cleanup(struct SqshFileReader *context) {
	sqsh__buffer_cleanup(&context->buffer);
	sqsh__file_iterator_cleanup(&context->iterator);
	return 0;
}

int
sqsh_file_reader_free(struct SqshFileReader *context) {
	if (context == NULL) {
		return 0;
	}
	int rv = sqsh__file_reader_cleanup(context);
	free(context);
	return rv;
}
