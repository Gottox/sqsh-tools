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
 * @file         metablock_iterator.c
 */

#include <sqsh_extract_private.h>

#include <sqsh_archive.h>
#include <sqsh_data_private.h>
#include <sqsh_metablock_private.h>

#include <sqsh_common_private.h>
#include <sqsh_error.h>

static bool
metablock_iterator_next(void *iterator, size_t desired_size, int *err) {
	(void)desired_size;
	return sqsh__metablock_iterator_next(iterator, err);
}
static int
metablock_iterator_skip(
		void *iterator, sqsh_index_t *offset, size_t desired_size) {
	(void)desired_size;
	return sqsh__metablock_iterator_skip(iterator, offset);
}
static const uint8_t *
metablock_iterator_data(const void *iterator) {
	return sqsh__metablock_iterator_data(iterator);
}
static size_t
metablock_iterator_size(const void *iterator) {
	return sqsh__metablock_iterator_size(iterator);
}

static const struct SqshReaderIteratorImpl metablock_reader_impl = {
		.next = metablock_iterator_next,
		.skip = metablock_iterator_skip,
		.data = metablock_iterator_data,
		.size = metablock_iterator_size,
};

int
sqsh__metablock_reader_init(
		struct SqshMetablockReader *reader, struct SqshArchive *sqsh,
		const uint64_t start_address, const uint64_t upper_limit) {
	int rv;
	rv = sqsh__metablock_iterator_init(
			&reader->iterator, sqsh, start_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__reader_init(
			&reader->reader, &metablock_reader_impl, &reader->iterator);
out:
	return rv;
}

int
sqsh__metablock_reader_advance(
		struct SqshMetablockReader *reader, sqsh_index_t offset, size_t size) {
	return sqsh__reader_advance(&reader->reader, offset, size);
}

const uint8_t *
sqsh__metablock_reader_data(const struct SqshMetablockReader *reader) {
	return sqsh__reader_data(&reader->reader);
}

size_t
sqsh__metablock_reader_size(const struct SqshMetablockReader *reader) {
	return sqsh__reader_size(&reader->reader);
}

int
sqsh__metablock_reader_cleanup(struct SqshMetablockReader *reader) {
	sqsh__reader_cleanup(&reader->reader);
	sqsh__metablock_iterator_cleanup(&reader->iterator);

	return 0;
}
