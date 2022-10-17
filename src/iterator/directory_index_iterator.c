/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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

#include "directory_index_iterator.h"
#include "../context/inode_context.h"
#include "../data/inode_data.h"
#include "../error.h"

static const uint64_t INODE_HEADER_SIZE =
		SQSH_SIZEOF_INODE_HEADER + SQSH_SIZEOF_INODE_DIRECTORY_EXT;

static const struct SqshInode *
get_inode(const struct SqshInodeDirectoryIndexIterator *iterator) {
	return (const struct SqshInode *)sqsh_metablock_stream_data(
			&iterator->inode->metablock);
}

// TODO: use sqsh_data_inode_directory_ext_index().
static const struct SqshInodeDirectoryIndex *
current_directory_index(
		const struct SqshInodeDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)get_inode(iterator);
	return (const struct SqshInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
directory_index_data_more(
		struct SqshInodeDirectoryIndexIterator *iterator, size_t size) {
	return sqsh_metablock_stream_more(&iterator->inode->metablock, size);
}

int
sqsh_inode_directory_index_iterator_init(
		struct SqshInodeDirectoryIndexIterator *iterator,
		struct SqshInodeContext *inode) {
	int rv = 0;

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_DIRECTORY ||
		sqsh_inode_is_extended(inode) == false) {
		return -SQSH_ERROR_NO_EXTENDED_DIRECTORY;
	}

	iterator->inode = inode;
	iterator->current_offset = 0;
	iterator->next_offset = INODE_HEADER_SIZE;

	const struct SqshInodeDirectoryExt *xdir =
			sqsh_data_inode_directory_ext(get_inode(iterator));
	iterator->remaining_entries =
			sqsh_data_inode_directory_ext_index_count(xdir);
	return rv;
}

int
sqsh_inode_directory_index_iterator_next(
		struct SqshInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	iterator->current_offset = iterator->next_offset;

	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += SQSH_SIZEOF_INODE_DIRECTORY_INDEX;
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure current index has its name populated
	iterator->next_offset +=
			sqsh_inode_directory_index_iterator_name_size(iterator);
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return iterator->remaining_entries;
}

uint32_t
sqsh_inode_directory_index_iterator_index(
		struct SqshInodeDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_index(current);
}
uint32_t
sqsh_inode_directory_index_iterator_start(
		struct SqshInodeDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_start(current);
}
uint32_t
sqsh_inode_directory_index_iterator_name_size(
		struct SqshInodeDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_name_size(current) + 1;
}
const char *
sqsh_inode_directory_index_iterator_name(
		struct SqshInodeDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)sqsh_data_inode_directory_index_name(current);
}

int
sqsh_inode_directory_index_iterator_clean(
		struct SqshInodeDirectoryIndexIterator __attribute__((unused)) *
		iterator) {
	return 0;
}
