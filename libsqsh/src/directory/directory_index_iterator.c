/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         directory_index_iterator.c
 */

#include <sqsh_directory_private.h>

#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <stdlib.h>

static const uint64_t INODE_HEADER_SIZE = sizeof(struct SqshDataInodeHeader) +
		sizeof(struct SqshDataInodeDirectoryExt);

static const struct SqshDataInodeDirectoryIndex *
get_directory_index(const struct SqshDirectoryIndexIterator *iterator) {
	const uint8_t *data =
			sqsh__metablock_reader_data(&iterator->file.metablock);
	return (const struct SqshDataInodeDirectoryIndex *)data;
}

static const struct SqshDataInode *
get_inode(const struct SqshDirectoryIndexIterator *iterator) {
	const uint8_t *data =
			sqsh__metablock_reader_data(&iterator->file.metablock);
	return (const struct SqshDataInode *)data;
}

int
sqsh__directory_index_iterator_init(
		struct SqshDirectoryIndexIterator *iterator, struct SqshArchive *sqsh,
		uint64_t inode_ref) {
	int rv;
	struct SqshFile *file = &iterator->file;

	rv = sqsh__file_init(file, sqsh, inode_ref);
	if (rv < 0) {
		goto out;
	}

	if (sqsh_file_type(file) != SQSH_FILE_TYPE_DIRECTORY ||
		sqsh_file_is_extended(file) == false) {
		rv = -SQSH_ERROR_NO_EXTENDED_DIRECTORY;
		goto out;
	}

	iterator->next_offset = INODE_HEADER_SIZE;

	const struct SqshDataInodeDirectoryExt *xdir =
			sqsh__data_inode_directory_ext(get_inode(iterator));
	iterator->remaining_entries =
			sqsh__data_inode_directory_ext_index_count(xdir);

out:
	if (rv < 0) {
		sqsh__directory_index_iterator_cleanup(iterator);
	}
	return rv;
}

bool
sqsh__directory_index_iterator_next(
		struct SqshDirectoryIndexIterator *iterator, int *err) {
	int rv = 0;
	size_t size;
	bool has_next = false;

	if (iterator->remaining_entries == 0) {
		return 0;
	}

	const size_t remaining_entries = iterator->remaining_entries;
	iterator->remaining_entries--;

	/* Make sure next entry is loaded: */
	size = sizeof(struct SqshDataInodeDirectoryIndex);
	rv = sqsh__metablock_reader_advance(
			&iterator->file.metablock, iterator->next_offset, size);
	if (rv < 0) {
		goto out;
	}

	/* Make sure current index has its name populated */
	size += sqsh__directory_index_iterator_name_size(iterator);
	rv = sqsh__metablock_reader_advance(&iterator->file.metablock, 0, size);
	if (rv < 0) {
		goto out;
	}

	iterator->next_offset = size;
	has_next = remaining_entries > 0;
out:
	if (err != NULL) {
		*err = rv;
	}
	return has_next;
}

uint32_t
sqsh__directory_index_iterator_index(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshDataInodeDirectoryIndex *current =
			get_directory_index(iterator);
	return sqsh__data_inode_directory_index_index(current);
}
uint32_t
sqsh__directory_index_iterator_start(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshDataInodeDirectoryIndex *current =
			get_directory_index(iterator);
	return sqsh__data_inode_directory_index_start(current);
}
uint32_t
sqsh__directory_index_iterator_name_size(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshDataInodeDirectoryIndex *current =
			get_directory_index(iterator);
	return sqsh__data_inode_directory_index_name_size(current) + 1;
}
const char *
sqsh__directory_index_iterator_name(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshDataInodeDirectoryIndex *current =
			get_directory_index(iterator);
	return (const char *)sqsh__data_inode_directory_index_name(current);
}

int
sqsh__directory_index_iterator_cleanup(
		struct SqshDirectoryIndexIterator *iterator) {
	sqsh__file_cleanup(&iterator->file);
	return 0;
}
