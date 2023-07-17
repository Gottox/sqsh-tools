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
#include "../utils/utils.h"
#include <stdint.h>

#define BLOCK_INDEX_FINISHED UINT32_MAX

static const uint8_t ZERO_BLOCK[16384] = {0};

int
sqsh__file_iterator_init(
		struct SqshFileIterator *iterator, const struct SqshInode *inode) {
	int rv = 0;
	struct SqshArchive *archive = inode->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(archive);
	uint64_t block_address = sqsh_inode_file_blocks_start(inode);
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);
	rv = sqsh__archive_data_extract_manager(
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
	iterator->block_size = sqsh_superblock_block_size(superblock);
	iterator->inode = inode;
	iterator->sparse_size = 0;
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
	struct SqshExtractManager *compression_manager =
			iterator->compression_manager;
	const struct SqshInode *inode = iterator->inode;
	struct SqshExtractView *extract_view = &iterator->extract_view;
	const sqsh_index_t block_index = iterator->block_index;
	const sqsh_index_t data_block_size =
			sqsh_inode_file_block_size(inode, block_index);

	rv = sqsh__map_reader_advance(
			&iterator->map_reader, next_offset, data_block_size);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__extract_view_init(
			extract_view, compression_manager, &iterator->map_reader);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__extract_view_data(extract_view);
	iterator->size = sqsh__extract_view_size(extract_view);

	if (SQSH_ADD_OVERFLOW(block_index, 1, &iterator->block_index)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = sqsh_file_iterator_size(iterator);
out:
	return rv;
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
	for (; iterator->sparse_size == 0 && block_index < block_count;
		 block_index++) {
		if (sqsh_inode_file_block_is_compressed(inode, block_index)) {
			break;
		}
		if (outer_size >= desired_size) {
			break;
		}
		const uint32_t data_block_size =
				sqsh_inode_file_block_size(inode, block_index);
		/* Set the sparse size only if we are not at the last block. */
		if (block_index + 1 != block_count) {
			iterator->sparse_size = iterator->block_size - data_block_size;
		}

		uint64_t new_outer_size;
		if (SQSH_ADD_OVERFLOW(outer_size, data_block_size, &new_outer_size)) {
			rv = -SQSH_ERROR_INTEGER_OVERFLOW;
			goto out;
		}

		/* To avoid crossing mem block boundaries, we stop
		 * if the next block would cross the boundary. The only exception
		 * is that we need to map at least one block.
		 */
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
	iterator->size = outer_size;
	iterator->block_index = block_index;

	rv = 1;
out:
	return rv;
}

static int
map_zero_block(struct SqshFileIterator *iterator) {
	const struct SqshInode *inode = iterator->inode;

	const sqsh_index_t block_index = iterator->block_index;
	const bool has_fragment = sqsh_inode_file_has_fragment(inode);
	const size_t block_count = sqsh_inode_file_block_count(inode);
	const size_t file_size = sqsh_inode_file_size(inode);
	const size_t sparse_size = iterator->sparse_size;

	size_t size;
	if (file_size == 0) {
		size = 0;
	} else if (sparse_size > 0) {
		size = SQSH_MIN(sizeof(ZERO_BLOCK), sparse_size);
		iterator->sparse_size -= size;
	} else if (has_fragment || block_index != block_count - 1) {
		size = iterator->block_size;
	} else {
		size = file_size % iterator->block_size;
		if (size == 0) {
			size = iterator->block_size;
		}
	}

	iterator->size = size;
	iterator->data = ZERO_BLOCK;

	return size;
}

static int
map_block(struct SqshFileIterator *iterator, size_t desired_size) {
	int rv = 0;
	const struct SqshInode *inode = iterator->inode;

	const sqsh_index_t block_index = iterator->block_index;
	const bool is_compressed =
			sqsh_inode_file_block_is_compressed(inode, block_index);
	const size_t data_block_size =
			sqsh_inode_file_block_size(inode, block_index);
	const sqsh_index_t next_offset =
			sqsh__map_reader_size(&iterator->map_reader);

	if (data_block_size == 0) {
		rv = map_zero_block(iterator);
		iterator->block_index++;
	} else if (is_compressed) {
		rv = map_block_compressed(iterator, next_offset);
	} else {
		rv = map_block_uncompressed(iterator, next_offset, desired_size);
	}
	return rv;
}

static int
map_fragment(struct SqshFileIterator *iterator) {
	int rv = 0;
	const struct SqshInode *inode = iterator->inode;
	struct SqshArchive *archive = inode->archive;
	struct SqshFragmentTable *fragment_table = NULL;
	struct SqshFragmentView *fragment_view = &iterator->fragment_view;
	rv = sqsh_archive_fragment_table(archive, &fragment_table);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__fragment_view_init(fragment_view, inode);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__fragment_view_data(fragment_view);
	iterator->size = sqsh__fragment_view_size(fragment_view);
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

	if (iterator->sparse_size > 0) {
		return map_zero_block(iterator);
	} else if (iterator->block_index < block_count) {
		return map_block(iterator, desired_size);
	} else if (has_fragment && iterator->block_index == block_count) {
		return map_fragment(iterator);
	} else {
		iterator->data = NULL;
		iterator->size = 0;
		return 0;
	}
}

int
sqsh_file_iterator_skip(
		struct SqshFileIterator *iterator, size_t amount, size_t desired_size) {
	int rv = 0;
	if (amount == 0) {
		return 0;
	}

	for (size_t i = 0; i < amount - 1; i++) {
		int rv = sqsh_file_iterator_next(iterator, 1);
		if (rv < 0) {
			goto out;
		} else if (rv == 0) {
			rv = -SQSH_ERROR_OUT_OF_BOUNDS;
			goto out;
		}
	}

	rv = sqsh_file_iterator_next(iterator, desired_size);
	if (rv < 0) {
		goto out;
	} else if (rv == 0) {
		rv = -SQSH_ERROR_OUT_OF_BOUNDS;
		goto out;
	}
out:
	return rv;

#if 0
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
#endif
}

const uint8_t *
sqsh_file_iterator_data(const struct SqshFileIterator *iterator) {
	return iterator->data;
}

size_t
sqsh__file_iterator_block_size(const struct SqshFileIterator *iterator) {
	return iterator->block_size;
}

size_t
sqsh_file_iterator_size(const struct SqshFileIterator *iterator) {
	return iterator->size;
}

int
sqsh__file_iterator_cleanup(struct SqshFileIterator *iterator) {
	sqsh__map_reader_cleanup(&iterator->map_reader);
	sqsh__extract_view_cleanup(&iterator->extract_view);
	sqsh__fragment_view_cleanup(&iterator->fragment_view);
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
