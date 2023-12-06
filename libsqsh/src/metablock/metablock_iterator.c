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

#include <sqsh_metablock_private.h>

#include <sqsh_archive.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <stdint.h>
#include <string.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_extract_private.h>

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

static int
process_next_header(struct SqshMetablockIterator *iterator) {
	int rv = 0;

	rv = sqsh__map_reader_advance(
			&iterator->reader, iterator->outer_size,
			sizeof(struct SqshDataMetablock));
	if (rv < 0) {
		goto out;
	}

	const struct SqshDataMetablock *metablock =
			(struct SqshDataMetablock *)sqsh__map_reader_data(
					&iterator->reader);
	const bool is_compressed = sqsh__data_metablock_is_compressed(metablock);
	const uint16_t outer_size = sqsh__data_metablock_size(metablock);
	if (outer_size > SQSH_METABLOCK_BLOCK_SIZE) {
		rv = -SQSH_ERROR_SIZE_MISMATCH;
		goto out;
	}
	iterator->is_compressed = is_compressed;
	iterator->outer_size = outer_size;

out:
	return rv;
}

static int
map_data(struct SqshMetablockIterator *iterator) {
	int rv = 0;

	rv = sqsh__map_reader_advance(
			&iterator->reader, sizeof(struct SqshDataMetablock),
			iterator->outer_size);
	if (rv < 0) {
		goto out;
	}

	sqsh__extract_view_cleanup(&iterator->extract_view);
	if (iterator->is_compressed) {
		rv = sqsh__extract_view_init(
				&iterator->extract_view, iterator->compression_manager,
				&iterator->reader);
		if (rv < 0) {
			goto out;
		}
		iterator->data = sqsh__extract_view_data(&iterator->extract_view);
		size_t extracted_size =
				sqsh__extract_view_size(&iterator->extract_view);
		/* extracted_size is guaranteed to be less or equal to
		 * SQSH_METABLOCK_BLOCK_SIZE as the compression_manager does only
		 * provide a buffer of that size. If the inflated data is bigger, the
		 * compression itself will fail. Therefore this explicit typecast is
		 * safe.
		 */
		iterator->inner_size = (uint16_t)extracted_size;
	} else {
		iterator->data = sqsh__map_reader_data(&iterator->reader);
		iterator->inner_size = iterator->outer_size;
	}

out:
	return rv;
}

bool
sqsh__metablock_iterator_next(
		struct SqshMetablockIterator *iterator, int *err) {
	int rv = 0;

	rv = process_next_header(iterator);
	if (rv < 0) {
		goto out;
	}

	rv = map_data(iterator);
out:
	if (err != NULL) {
		*err = rv;
	}
	/* metablock iterators have no way to see if there is a next block. this
	 * needs to be by the caller.
	 */
	return rv == 0;
}

int
sqsh__metablock_iterator_skip(
		struct SqshMetablockIterator *iterator, sqsh_index_t *offset) {
	int rv = 0;

	size_t current_size = sqsh__metablock_iterator_size(iterator);
	if (*offset < current_size) {
		return 0;
	}

	for (;;) {
		*offset -= current_size;
		current_size = SQSH_METABLOCK_BLOCK_SIZE;
		rv = process_next_header(iterator);
		if (rv < 0) {
			goto out;
		}
		if (*offset < current_size) {
			break;
		}

		sqsh_index_t skip_size;
		if (SQSH_ADD_OVERFLOW(
					iterator->outer_size, sizeof(struct SqshDataMetablock),
					&skip_size)) {
			rv = -SQSH_ERROR_OUT_OF_BOUNDS;
			goto out;
		}
		rv = sqsh__map_reader_advance(&iterator->reader, skip_size, 0);
		if (rv < 0) {
			goto out;
		}
		iterator->outer_size = 0;
	}

	rv = map_data(iterator);
	if (rv < 0) {
		goto out;
	}

	/* When the iterator is skipped, we assume that the offset is within the
	 * logical size of a series of metablocks of the maximal metablock size.
	 * We cannot check this, because that would require us to decompress every
	 * metablock in the series. So we just sanity check the last metablock
	 * and assume that the caller does the sanity checks.
	 */
	if (*offset >= sqsh__metablock_iterator_size(iterator)) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}

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
