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

static int
file_iterator_next(void *iterator, size_t desired_size) {
	return sqsh_file_iterator_next(iterator, desired_size);
}
static int
file_iterator_skip(void *iterator, size_t amount, size_t desired_size) {
	return sqsh_file_iterator_skip(iterator, amount, desired_size);
}
static size_t
file_iterator_block_size(const void *iterator) {
	return sqsh_file_iterator_block_size(iterator);
}
static const uint8_t *
file_iterator_data(const void *iterator) {
	return sqsh_file_iterator_data(iterator);
}
static size_t
file_iterator_size(const void *iterator) {
	return sqsh_file_iterator_size(iterator);
}

static const struct SqshIteratorImpl file_reader_impl = {
		.next = file_iterator_next,
		.skip = file_iterator_skip,
		.block_size = file_iterator_block_size,
		.data = file_iterator_data,
		.size = file_iterator_size,
};

int
sqsh__file_reader_init(
		struct SqshFileReader *reader, const struct SqshInode *inode) {
	int rv = 0;
	rv = sqsh__file_iterator_init(&reader->iterator, inode);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__reader_init(
			&reader->reader, &file_reader_impl, &reader->iterator);
out:
	return rv;
}

struct SqshFileReader *
sqsh_file_reader_new(const struct SqshInode *inode, int *err) {
	int rv = 0;
	struct SqshFileReader *reader = calloc(1, sizeof(struct SqshFileReader));
	if (reader == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = sqsh__file_reader_init(reader, inode);
	if (rv < 0) {
		free(reader);
		reader = NULL;
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	return reader;
}

int
sqsh_file_reader_advance(
		struct SqshFileReader *reader, sqsh_index_t offset, size_t size) {
	return sqsh__reader_advance(&reader->reader, offset, size);
}

const uint8_t *
sqsh_file_reader_data(const struct SqshFileReader *reader) {
	return sqsh__reader_data(&reader->reader);
}

size_t
sqsh_file_reader_size(const struct SqshFileReader *reader) {
	return sqsh__reader_size(&reader->reader);
}

int
sqsh__file_reader_cleanup(struct SqshFileReader *reader) {
	sqsh__reader_cleanup(&reader->reader);
	sqsh__file_iterator_cleanup(&reader->iterator);

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
