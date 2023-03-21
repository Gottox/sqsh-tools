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
#include "utils.h"

SQSH_NO_UNUSED static sqsh_index_t
find_block_address(
		const struct SqshInodeContext *inode, sqsh_index_t block_index) {
	uint64_t block_address = sqsh_inode_file_blocks_start(inode);
	for (sqsh_index_t i = 0; i < block_index; i++) {
		block_address += sqsh_inode_file_block_size(inode, i);
	}
	return block_address;
}

int
sqsh__file_reader_init(
		struct SqshFileReader *reader, const struct SqshInodeContext *inode) {
	reader->inode = inode;
	reader->compression = sqsh_archive_compression_data(inode->sqsh);
	reader->superblock = sqsh_archive_superblock(inode->sqsh);
	reader->map_manager = sqsh_archive_map_manager(inode->sqsh);

	return 0;
}

struct SqshFileReader *
sqsh_file_reader_new(const struct SqshInodeContext *inode, int *err) {
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

static bool
is_direct(
		const sqsh_index_t block_index, const size_t size,
		const struct SqshInodeContext *inode,
		const struct SqshSuperblockContext *superblock) {
	const uint64_t block_size = sqsh_superblock_block_size(superblock);

	const size_t end_block_index = block_index + size / block_size;
	if (end_block_index >= sqsh_inode_file_block_count(inode)) {
		return false;
	}
	for (sqsh_index_t i = block_index; i <= end_block_index; i++) {
		if (sqsh_inode_file_block_is_compressed(inode, i)) {
			return false;
		}
	}
	return true;
}

SQSH_NO_UNUSED static int
file_map_fragment(
		struct SqshFileReader *reader, sqsh_index_t block_offset, size_t size) {
	const struct SqshInodeContext *inode = reader->inode;
	struct SqshArchive *archive = inode->sqsh;
	struct SqshFragmentTable *fragment_table;
	sqsh_index_t end_offset;
	int rv;

	if (sqsh_inode_file_has_fragment(inode) == false) {
		// rv = -SQSH_ERROR_INVALID_FILE;
		rv = -SQSH_ERROR_TODO;
		goto out;
	}

	rv = sqsh_archive_fragment_table(archive, &fragment_table);
	if (rv < 0) {
		goto out;
	}

	sqsh__buffer_drain(&reader->buffer);
	sqsh__map_reader_cleanup(&reader->map_reader);

	// TODO: implement direct mapping to the fragment table.
	rv = sqsh_fragment_table_to_buffer(fragment_table, inode, &reader->buffer);
	if (rv < 0) {
		goto out;
	}

	if (SQSH_ADD_OVERFLOW(block_offset, size, &end_offset)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	reader->data = sqsh__buffer_data(&reader->buffer) + block_offset;
	reader->data_size = end_offset - block_offset;

out:
	return rv;
}

SQSH_NO_UNUSED static int
file_map_direct(
		struct SqshFileReader *reader, sqsh_index_t block_index,
		sqsh_index_t block_offset, size_t size) {
	int rv;
	struct SqshMapManager *map_manager = reader->map_manager;
	const struct SqshInodeContext *inode = reader->inode;
	const uint64_t block_address = find_block_address(inode, block_index);
	const struct SqshSuperblockContext *superblock =
			sqsh_archive_superblock(inode->sqsh);
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);

	sqsh__buffer_drain(&reader->buffer);
	sqsh__map_reader_cleanup(&reader->map_reader);

	rv = sqsh__map_reader_init(
			&reader->map_reader, map_manager, block_address, upper_limit);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__map_reader_advance(&reader->map_reader, block_offset, size);
	if (rv < 0) {
		goto out;
	}

	reader->data = sqsh__map_reader_data(&reader->map_reader);
	reader->data_size = size;

out:
	return rv;
}

SQSH_NO_UNUSED static int
file_map_buffered(
		struct SqshFileReader *reader, sqsh_index_t block_index,
		sqsh_index_t block_offset, size_t size) {
	int rv;
	struct SqshMapManager *map_manager = reader->map_manager;
	const struct SqshInodeContext *inode = reader->inode;

	// TODO: use getter instead of accessing the field directly.
	struct SqshCompressionManager *compression_manager;
	rv = sqsh__archive_file_compression_manager(
			inode->sqsh, &compression_manager);
	if (rv < 0) {
		goto out;
	}

	uint64_t block_address = find_block_address(inode, block_index);
	const struct SqshSuperblockContext *superblock =
			sqsh_archive_superblock(inode->sqsh);
	// TODO: sane upper limit
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);
	const size_t target_size = size + block_offset;

	sqsh__buffer_drain(&reader->buffer);
	sqsh__map_reader_cleanup(&reader->map_reader);

	rv = sqsh__map_reader_init(
			&reader->map_reader, map_manager, block_address, upper_limit);
	if (rv < 0) {
		goto out;
	}

	const uint32_t block_count = sqsh_inode_file_block_count(inode);
	for (; block_index < block_count &&
		 sqsh__buffer_size(&reader->buffer) < target_size;
		 block_index++) {
		const bool is_compressed =
				sqsh_inode_file_block_is_compressed(inode, block_index);
		const uint32_t block_size =
				sqsh_inode_file_block_size(inode, block_index);
		rv = sqsh__map_reader_advance(
				&reader->map_reader, block_offset, block_size);

		if (is_compressed) {
			const struct SqshBuffer *uncompressed;
			rv = sqsh__compression_manager_get(
					compression_manager, block_address, block_size,
					&uncompressed);
			if (rv < 0) {
				goto out;
			}

			const uint8_t *buffer_data = sqsh__buffer_data(uncompressed);
			const size_t buffer_size = sqsh__buffer_size(uncompressed);

			rv = sqsh__buffer_append(&reader->buffer, buffer_data, buffer_size);
			if (rv < 0) {
				goto out;
			}
			sqsh__compression_manager_release(
					compression_manager, uncompressed);
		} else {
			rv = sqsh__buffer_append(
					&reader->buffer, sqsh__map_reader_data(&reader->map_reader),
					block_size);
			if (rv < 0) {
				goto out;
			}
		}
		block_address += block_size;
	}

	if (sqsh__buffer_size(&reader->buffer) < target_size) {
		if (sqsh_inode_file_has_fragment(inode)) {
			struct SqshArchive *archive = inode->sqsh;
			struct SqshFragmentTable *fragment_table;

			rv = sqsh_archive_fragment_table(archive, &fragment_table);
			if (rv < 0) {
				goto out;
			}

			rv = sqsh_fragment_table_to_buffer(
					fragment_table, inode, &reader->buffer);
			if (rv < 0) {
				goto out;
			}
		} else {
			// rv = -SQSH_ERROR_INVALID_FILE;
			rv = -SQSH_ERROR_TODO;
			goto out;
		}
	}

	const uint8_t *buffer_data = sqsh__buffer_data(&reader->buffer);
	reader->data = &buffer_data[block_offset];
	reader->data_size = size;

out:
	return rv;
}

static int
check_bounds(
		sqsh_index_t block_index, uint64_t block_offset, size_t size,
		const struct SqshInodeContext *inode,
		const struct SqshSuperblockContext *superblock) {
	const uint32_t block_size = sqsh_superblock_block_size(superblock);
	const uint64_t file_size = sqsh_inode_file_size(inode);

	uint64_t content_begin_offset;
	uint64_t content_end_offset;
	if (SQSH_MULT_OVERFLOW(block_index, block_size, &content_begin_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (SQSH_ADD_OVERFLOW(
				content_begin_offset, block_offset, &content_begin_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	if (SQSH_ADD_OVERFLOW(content_begin_offset, size, &content_end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	if (content_end_offset > file_size) {
		return -SQSH_ERROR_TODO;
	}
	return 0;
}

int
sqsh_file_reader_advance(
		struct SqshFileReader *reader, sqsh_index_t offset, size_t size) {
	int rv = 0;
	const struct SqshSuperblockContext *superblock = reader->superblock;
	const struct SqshInodeContext *inode = reader->inode;
	const uint32_t block_size = sqsh_superblock_block_size(superblock);
	uint64_t block_offset = reader->datablock_offset;
	uint64_t block_index = reader->datablock_index;

	block_offset += offset;
	block_index += block_offset / block_size;
	block_offset %= block_size;

	reader->datablock_index = block_index;
	reader->datablock_offset = block_offset;

	rv = check_bounds(block_index, block_offset, size, inode, superblock);
	if (rv < 0) {
		return rv;
	}

	if (block_index >= sqsh_inode_file_block_count(inode)) {
		return file_map_fragment(reader, block_offset, size);
	}

	if (is_direct(block_index, size, inode, superblock)) {
		return file_map_direct(reader, block_index, block_offset, size);
	} else {
		return file_map_buffered(reader, block_index, block_offset, size);
	}
}

const uint8_t *
sqsh_file_reader_data(struct SqshFileReader *reader) {
	return reader->data;
}

size_t
sqsh_file_reader_size(struct SqshFileReader *reader) {
	return reader->data_size;
}

int
sqsh__file_reader_cleanup(struct SqshFileReader *context) {
	sqsh__buffer_cleanup(&context->buffer);
	sqsh__map_reader_cleanup(&context->map_reader);
	context->data = NULL;
	context->data_size = 0;
	context->inode = NULL;
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
