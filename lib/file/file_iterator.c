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
#include "utils.h"

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
	iterator->block_offset = 0;

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
map_block(struct SqshFileIterator *iterator) {
	int rv = 0;
	struct SqshCompressionManager *compression_manager =
			iterator->compression_manager;
	struct SqshMapManager *map_manager = iterator->map_manager;
	const struct SqshInodeContext *inode = iterator->inode;
	const struct SqshBuffer *compressed = NULL;
	const struct SqshMapSlice *uncompressed = NULL;

	const uint64_t start_address = sqsh_inode_file_blocks_start(inode);
	const sqsh_index_t block_offset = iterator->block_offset;
	const sqsh_index_t block_index = iterator->block_index;
	const uint32_t outer_size = sqsh_inode_file_block_size(inode, block_index);
	const bool is_compressed =
			sqsh_inode_file_block_is_compressed(inode, block_index);
	uint64_t block_address;
	if (SQSH_ADD_OVERFLOW(start_address, block_offset, &block_address)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}

	sqsh__map_manager_release(map_manager, iterator->current_uncompressed);
	sqsh__compression_manager_release(
			compression_manager, iterator->current_compressed);

	if (is_compressed) {
		rv = sqsh__compression_manager_get(
				compression_manager, block_address, outer_size, &compressed);
		if (rv < 0) {
			goto out;
		}
		iterator->data = sqsh__buffer_data(compressed);
		iterator->data_size = sqsh__buffer_size(compressed);

	} else {
		rv = sqsh__map_manager_get(
				map_manager, block_address, 1, &uncompressed);
		if (rv < 0) {
			goto out;
		}
		iterator->data = sqsh__map_slice_data(uncompressed);
		iterator->data_size = outer_size;
	}
	iterator->current_uncompressed = uncompressed;
	iterator->current_compressed = compressed;

	if (SQSH_ADD_OVERFLOW(block_index, 1, &iterator->block_index)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	if (SQSH_ADD_OVERFLOW(block_offset, outer_size, &iterator->block_offset)) {
		rv = SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

out:
	return 1;
}

static int
map_tail(struct SqshFileIterator *iterator) {
	(void)iterator;
	iterator->data = NULL;
	iterator->data_size = 0;
	return 0;
}

int
sqsh_file_iterator_next(struct SqshFileIterator *iterator) {
	size_t block_count = sqsh_inode_file_block_count(iterator->inode);

	if (iterator->block_index < block_count) {
		return map_block(iterator);
	} else {
		return map_tail(iterator);
	}

	return 0;
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
	sqsh__map_manager_release(iterator->map_manager, iterator->current_uncompressed);
	sqsh__compression_manager_release(
			iterator->compression_manager, iterator->current_compressed);
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
