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

#include "../../include/sqsh_metablock_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_error.h"

#include <stdint.h>
#include <string.h>

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_extract_private.h"

int
sqsh__metablock_iterator_init(
		struct SqshMetablockIterator *iterator, struct SqshArchive *sqsh,
		uint64_t start_address, uint64_t upper_limit) {
	int rv = 0;
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(sqsh);

	iterator->outer_size = 0;
	iterator->inner_size = 0;
	iterator->compression_manager =
			sqsh__archive_metablock_extract_manager(sqsh);
	memset(&iterator->extract_view, 0, sizeof(iterator->extract_view));
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_reader_init(
			&iterator->reader, map_manager, start_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

int
sqsh__metablock_iterator_next(struct SqshMetablockIterator *iterator) {
	int rv = 0;

	sqsh__extract_view_cleanup(&iterator->extract_view);

	rv = sqsh__map_reader_advance(
			&iterator->reader, iterator->outer_size, SQSH_SIZEOF_METABLOCK);
	if (rv < 0) {
		goto out;
	}

	const struct SqshDataMetablock *metablock =
			(struct SqshDataMetablock *)sqsh__map_reader_data(
					&iterator->reader);
	const bool is_compressed = sqsh__data_metablock_is_compressed(metablock);
	const size_t outer_size = sqsh__data_metablock_size(metablock);
	if (outer_size > SQSH_METABLOCK_BLOCK_SIZE) {
		rv = -SQSH_ERROR_SIZE_MISMATCH;
		goto out;
	}
	iterator->outer_size = outer_size;

	if (iterator->outer_size > SQSH_METABLOCK_BLOCK_SIZE) {
		rv = -SQSH_ERROR_SIZE_MISMATCH;
		goto out;
	}

	rv = sqsh__map_reader_advance(
			&iterator->reader, SQSH_SIZEOF_METABLOCK, iterator->outer_size);
	if (rv < 0) {
		goto out;
	}

	if (is_compressed) {
		rv = sqsh__extract_view_init(
				&iterator->extract_view, iterator->compression_manager,
				&iterator->reader);
		if (rv < 0) {
			goto out;
		}
		iterator->data = sqsh__extract_view_data(&iterator->extract_view);
		iterator->inner_size = sqsh__extract_view_size(&iterator->extract_view);
	} else {
		iterator->data = sqsh__map_reader_data(&iterator->reader);
		iterator->inner_size = iterator->outer_size;
	}
	rv = sqsh__metablock_iterator_size(iterator);
out:
	return rv;
}

const uint8_t *
sqsh__metablock_iterator_data(const struct SqshMetablockIterator *iterator) {
	return iterator->data;
}

size_t
sqsh__metablock_iterator_size(const struct SqshMetablockIterator *iterator) {
	return iterator->inner_size;
}

int
sqsh__metablock_iterator_cleanup(struct SqshMetablockIterator *iterator) {
	sqsh__extract_view_cleanup(&iterator->extract_view);
	return sqsh__map_reader_cleanup(&iterator->reader);
}
