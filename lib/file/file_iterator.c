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
 * @file         file_iterator.c
 */

#include "../../include/sqsh_file_private.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_primitive_private.h"

#include "../../include/sqsh_inode_private.h"
#include "../utils.h"
#include <stdint.h>

#define BLOCK_INDEX_FINISHED UINT32_MAX

int
sqsh__file_iterator_init(
		struct SqshFileIterator *iterator, const struct SqshInode *inode) {
	int rv = 0;
	struct SqshArchive *archive = inode->sqsh;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(archive);
	uint64_t block_address = sqsh_inode_file_blocks_start(inode);
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);
	rv = sqsh__archive_file_compression_manager(
			archive, &iterator->compression_manager);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_reader_init(
			&iterator->map_reader, map_manager, block_address, upper_limit);
	if (rv < 0) {
		goto out;
	}

	iterator->block_index = 0;
	iterator->inode = inode;
out:
	return rv;
}

struct SqshFileIterator *
sqsh_file_iterator_new(const struct SqshInode *inode, int *err) {
	struct SqshFileIterator *iterator =
			calloc(1, sizeof(struct SqshFileIterator));
	if (iterator == NULL) {
		return NULL;
	}
	*err = sqsh__file_iterator_init(iterator, inode);
	if (*err < 0) {
		free(iterator);
		return NULL;
	}
	return iterator;
}

static int
map_block_compressed(
		struct SqshFileIterator *iterator, sqsh_index_t next_offset) {
	int rv = 0;
	struct SqshCompressionManager *compression_manager =
			iterator->compression_manager;
	const struct SqshInode *inode = iterator->inode;
	struct SqshExtractView *extract_view = &iterator->extract_view;
	const sqsh_index_t block_index = iterator->block_index;
	const sqsh_index_t block_size =
			sqsh_inode_file_block_size(inode, block_index);

	rv = sqsh__map_reader_advance(
			&iterator->map_reader, next_offset, block_size);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__extract_view_init(extract_view, compression_manager, &iterator->map_reader);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__extract_view_data(extract_view);
	iterator->data_size = sqsh__extract_view_size(extract_view);

	if (SQSH_ADD_OVERFLOW(block_index, 1, &iterator->block_index)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

out:
	return 1;
}

static int
map_block_uncompressed(
		struct SqshFileIterator *iterator, sqsh_index_t next_offset,
		size_t desired_size) {
	int rv = 0;
	const struct SqshInode *inode = iterator->inode;

	sqsh_index_t block_index = iterator->block_index;
	struct SqshMapReader *reader = &iterator->map_reader;
	if (rv < 0) {
		goto out;
	}

	const uint64_t block_count = sqsh_inode_file_block_count(inode);
	uint64_t outer_size = 0;
	const size_t remaining_direct = sqsh__map_reader_remaining_direct(reader);
	for (; block_index < block_count; block_index++) {
		if (sqsh_inode_file_block_is_compressed(inode, block_index)) {
			break;
		}
		if (outer_size >= desired_size) {
			break;
		}
		const uint32_t block_size =
				sqsh_inode_file_block_size(inode, block_index);

		uint64_t new_outer_size;
		if (SQSH_ADD_OVERFLOW(outer_size, block_size, &new_outer_size)) {
			rv = SQSH_ERROR_INTEGER_OVERFLOW;
			goto out;
		}

		// To avoid crossing mem block boundaries, we stop
		// if the next block would cross the boundary. The only exception
		// is that we need to map at least one block.
		if (new_outer_size > remaining_direct && outer_size > 0) {
			break;
		}
		outer_size = new_outer_size;
	}
	rv = sqsh__map_reader_advance(reader, next_offset, outer_size);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__map_reader_data(reader);
	iterator->data_size = outer_size;
	iterator->block_index = block_index;

	rv = 1;
out:
	return rv;
}

static int
map_block(struct SqshFileIterator *iterator, size_t desired_size) {
	const struct SqshInode *inode = iterator->inode;

	const sqsh_index_t block_index = iterator->block_index;
	const bool is_compressed =
			sqsh_inode_file_block_is_compressed(inode, block_index);
	const sqsh_index_t next_offset =
			sqsh__map_reader_size(&iterator->map_reader);

	if (is_compressed) {
		return map_block_compressed(iterator, next_offset);
	} else {
		return map_block_uncompressed(iterator, next_offset, desired_size);
	}
}

static int
map_fragment(struct SqshFileIterator *iterator) {
	int rv = 0;
	const struct SqshInode *inode = iterator->inode;
	struct SqshArchive *archive = inode->sqsh;
	struct SqshFragmentTable *fragment_table = NULL;
	struct SqshBuffer *fragment_buffer = &iterator->fragment_buffer;
	rv = sqsh_archive_fragment_table(archive, &fragment_table);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__buffer_init(fragment_buffer);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_fragment_table_to_buffer(fragment_table, inode, fragment_buffer);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__buffer_data(fragment_buffer);
	iterator->data_size = sqsh__buffer_size(fragment_buffer);
	iterator->block_index = BLOCK_INDEX_FINISHED;
out:
	if (rv < 0) {
		return rv;
	} else {
		return 1;
	}
}

int
sqsh_file_iterator_next(
		struct SqshFileIterator *iterator, size_t desired_size) {
	const struct SqshInode *inode = iterator->inode;
	size_t block_count = sqsh_inode_file_block_count(inode);
	const bool has_fragment = sqsh_inode_file_has_fragment(inode);

	sqsh__extract_view_cleanup(&iterator->extract_view);
	sqsh__buffer_cleanup(&iterator->fragment_buffer);

	if (iterator->block_index < block_count) {
		return map_block(iterator, desired_size);
	} else if (has_fragment && iterator->block_index == block_count) {
		return map_fragment(iterator);
	} else {
		iterator->data = NULL;
		iterator->data_size = 0;
		return 0;
	}
}

int
sqsh_file_iterator_skip(struct SqshFileIterator *iterator, size_t amount) {
	int rv = 0;
	const struct SqshInode *inode = iterator->inode;

	for (sqsh_index_t i = 0; i < amount; i++, iterator->block_index++) {
		const uint32_t block_size =
				sqsh_inode_file_block_size(inode, iterator->block_index);

		rv = sqsh__map_reader_advance(&iterator->map_reader, block_size, 0);
		if (rv < 0) {
			goto out;
		}
	}

out:
	return rv;
}

const uint8_t *
sqsh_file_iterator_data(struct SqshFileIterator *iterator) {
	return iterator->data;
}

size_t
sqsh_file_iterator_size(struct SqshFileIterator *iterator) {
	return iterator->data_size;
}

int
sqsh__file_iterator_cleanup(struct SqshFileIterator *iterator) {
	sqsh__map_reader_cleanup(&iterator->map_reader);
	sqsh__extract_view_cleanup(&iterator->extract_view);
	sqsh__buffer_cleanup(&iterator->fragment_buffer);
	return 0;
}

int
sqsh_file_iterator_free(struct SqshFileIterator *iterator) {
	if (iterator == NULL) {
		return 0;
	}
	int rv = sqsh__file_iterator_cleanup(iterator);
	free(iterator);
	return rv;
}
