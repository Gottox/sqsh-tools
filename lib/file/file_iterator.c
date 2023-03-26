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

#define BLOCK_INDEX_FINISHED UINT32_MAX

int
sqsh__file_iterator_init(
		struct SqshFileIterator *iterator,
		const struct SqshInodeContext *inode) {
	int rv = 0;
	struct SqshArchive *archive = inode->sqsh;
	iterator->inode = inode;
	iterator->superblock = sqsh_archive_superblock(archive);
	rv = sqsh__archive_file_compression_manager(
			archive, &iterator->compression_manager);
	if (rv < 0) {
		return rv;
	}
	iterator->map_manager = sqsh_archive_map_manager(archive);
	iterator->block_index = 0;
	iterator->block_address = sqsh_inode_file_blocks_start(inode);

	return rv;
}

struct SqshFileIterator *
sqsh_file_iterator_new(const struct SqshInodeContext *inode, int *err) {
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
map_block_compressed(struct SqshFileIterator *iterator) {
	int rv = 0;
	struct SqshCompressionManager *compression_manager =
			iterator->compression_manager;
	const struct SqshInodeContext *inode = iterator->inode;
	const struct SqshBuffer *compressed = NULL;

	const sqsh_index_t block_address = iterator->block_address;
	const sqsh_index_t block_index = iterator->block_index;
	const uint32_t outer_size = sqsh_inode_file_block_size(inode, block_index);

	rv = sqsh__compression_manager_get(
			compression_manager, block_address, outer_size, &compressed);
	if (rv < 0) {
		goto out;
	}
	iterator->data = sqsh__buffer_data(compressed);
	iterator->data_size = sqsh__buffer_size(compressed);

	iterator->current_compressed = compressed;

	if (SQSH_ADD_OVERFLOW(block_index, 1, &iterator->block_index)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	if (SQSH_ADD_OVERFLOW(
				block_address, outer_size, &iterator->block_address)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

out:
	return 1;
}

static int
map_block_uncompressed(struct SqshFileIterator *iterator, size_t desired_size) {
	// TODO
	(void)desired_size;
	struct SqshMapReader *map_reader = &iterator->current_uncompressed;
	int rv = 0;
	struct SqshMapManager *map_manager = iterator->map_manager;
	const struct SqshInodeContext *inode = iterator->inode;
	const struct SqshSuperblockContext *superblock =
			sqsh_archive_superblock(inode->sqsh);
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);

	const sqsh_index_t block_address = iterator->block_address;
	sqsh_index_t block_index = iterator->block_index;
	rv = sqsh__map_reader_init(
			map_reader, map_manager, block_address, upper_limit);
	if (rv < 0) {
		goto out;
	}

	const uint64_t block_count = sqsh_inode_file_block_count(inode);
	uint32_t outer_size = 0;
	for (; block_index < block_count; block_index++) {
		if (sqsh_inode_file_block_is_compressed(inode, block_index)) {
			break;
		}
		if (outer_size >= desired_size) {
			break;
		}

		if (SQSH_ADD_OVERFLOW(
					outer_size, sqsh_inode_file_block_size(inode, block_index),
					&outer_size)) {
			rv = SQSH_ERROR_INTEGER_OVERFLOW;
			goto out;
		}
	}
	rv = sqsh__map_reader_advance(map_reader, 0, outer_size);
	iterator->data = sqsh__map_reader_data(map_reader);
	iterator->data_size = outer_size;
	iterator->block_index = block_index;

	if (SQSH_ADD_OVERFLOW(
				block_address, outer_size, &iterator->block_address)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
out:
	return 1;
}

static int
map_block(struct SqshFileIterator *iterator, size_t desired_size) {
	const struct SqshInodeContext *inode = iterator->inode;

	const sqsh_index_t block_index = iterator->block_index;
	const bool is_compressed =
			sqsh_inode_file_block_is_compressed(inode, block_index);

	if (is_compressed) {
		return map_block_compressed(iterator);
	} else {
		return map_block_uncompressed(iterator, desired_size);
	}
}

static int
map_fragment(struct SqshFileIterator *iterator) {
	int rv = 0;
	const struct SqshInodeContext *inode = iterator->inode;
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
	const struct SqshInodeContext *inode = iterator->inode;
	size_t block_count = sqsh_inode_file_block_count(inode);
	const bool has_fragment = sqsh_inode_file_has_fragment(inode);
	struct SqshCompressionManager *compression_manager =
			iterator->compression_manager;

	sqsh__map_reader_cleanup(&iterator->current_uncompressed);
	sqsh__compression_manager_release(
			compression_manager, iterator->current_compressed);
	iterator->current_compressed = NULL;
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
	sqsh__map_reader_cleanup(&iterator->current_uncompressed);
	sqsh__compression_manager_release(
			iterator->compression_manager, iterator->current_compressed);

	iterator->current_compressed = NULL;
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
