/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory_index_iterator
 * @created     : Monday Dec 20, 2021 12:41:47 CET
 */

#include "directory_index_iterator.h"
#include "../context/inode_context.h"
#include "../data/inode.h"
#include "../error.h"

static const uint64_t INODE_HEADER_SIZE =
		HSQS_SIZEOF_INODE_HEADER + HSQS_SIZEOF_INODE_DIRECTORY_EXT;

static const struct HsqsInode *
get_inode(const struct HsqsInodeDirectoryIndexIterator *iterator) {
	return (const struct HsqsInode *)hsqs_metablock_stream_data(
			&iterator->inode->metablock);
}

// TODO: use hsqs_data_inode_directory_ext_index().
static const struct HsqsInodeDirectoryIndex *
current_directory_index(
		const struct HsqsInodeDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)get_inode(iterator);
	return (const struct HsqsInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
directory_index_data_more(
		struct HsqsInodeDirectoryIndexIterator *iterator, size_t size) {
	return hsqs_metablock_stream_more(&iterator->inode->metablock, size);
}

int
hsqs_inode_directory_index_iterator_init(
		struct HsqsInodeDirectoryIndexIterator *iterator,
		struct HsqsInodeContext *inode) {
	int rv = 0;

	if (hsqs_inode_type(inode) != HSQS_INODE_TYPE_DIRECTORY ||
		hsqs_inode_is_extended(inode) == false) {
		return -HSQS_ERROR_NO_EXTENDED_DIRECTORY;
	}

	iterator->inode = inode;
	iterator->current_offset = 0;
	iterator->next_offset = INODE_HEADER_SIZE;

	const struct HsqsInodeDirectoryExt *xdir =
			hsqs_data_inode_directory_ext(get_inode(iterator));
	iterator->remaining_entries =
			hsqs_data_inode_directory_ext_index_count(xdir);
	return rv;
}

int
hsqs_inode_directory_index_iterator_next(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	iterator->current_offset = iterator->next_offset;

	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += HSQS_SIZEOF_INODE_DIRECTORY_INDEX;
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure current index has its name populated
	iterator->next_offset +=
			hsqs_inode_directory_index_iterator_name_size(iterator);
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return iterator->remaining_entries;
}

uint32_t
hsqs_inode_directory_index_iterator_index(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_index(current);
}
uint32_t
hsqs_inode_directory_index_iterator_start(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_start(current);
}
uint32_t
hsqs_inode_directory_index_iterator_name_size(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_name_size(current) + 1;
}
const char *
hsqs_inode_directory_index_iterator_name(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)hsqs_data_inode_directory_index_name(current);
}

int
hsqs_inode_directory_index_iterator_clean(
		struct HsqsInodeDirectoryIndexIterator __attribute__((unused)) *
		iterator) {
	return 0;
}
