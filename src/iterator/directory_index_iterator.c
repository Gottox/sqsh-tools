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
 * @file         directory_index_iterator.c
 */

#include <sqsh.h>
#include <sqsh_data.h>
#include <sqsh_iterator_private.h>

static const uint64_t INODE_HEADER_SIZE =
		SQSH_SIZEOF_INODE_HEADER + SQSH_SIZEOF_INODE_DIRECTORY_EXT;

static const struct SqshInode *
get_inode(const struct SqshDirectoryIndexIterator *iterator) {
	return (const struct SqshInode *)sqsh__metablock_stream_data(
			&iterator->inode->metablock);
}

// TODO: use sqsh_data_inode_directory_ext_index().
static const struct SqshInodeDirectoryIndex *
current_directory_index(const struct SqshDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)get_inode(iterator);
	return (const struct SqshInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
directory_index_data_more(
		struct SqshDirectoryIndexIterator *iterator, size_t size) {
	return sqsh__metablock_stream_more(&iterator->inode->metablock, size);
}

int
sqsh__directory_index_iterator_init(
		struct SqshDirectoryIndexIterator *iterator,
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

SQSH_NO_UNUSED
struct SqshDirectoryIndexIterator *
sqsh__directory_index_iterator_new(struct SqshInodeContext *inode, int *err) {
	struct SqshDirectoryIndexIterator *iterator =
			calloc(1, sizeof(struct SqshDirectoryIndexIterator));
	if (iterator == NULL) {
		return NULL;
	}
	*err = sqsh__directory_index_iterator_init(iterator, inode);
	if (*err < 0) {
		free(iterator);
		return NULL;
	}
	return iterator;
}

int
sqsh__directory_index_iterator_next(
		struct SqshDirectoryIndexIterator *iterator) {
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
	iterator->next_offset += sqsh__directory_index_iterator_name_size(iterator);
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return iterator->remaining_entries;
}

uint32_t
sqsh__directory_index_iterator_index(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_index(current);
}
uint32_t
sqsh__directory_index_iterator_start(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_start(current);
}
uint32_t
sqsh__directory_index_iterator_name_size(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return sqsh_data_inode_directory_index_name_size(current) + 1;
}
const char *
sqsh__directory_index_iterator_name(
		const struct SqshDirectoryIndexIterator *iterator) {
	const struct SqshInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)sqsh_data_inode_directory_index_name(current);
}

int
sqsh__directory_index_iterator_cleanup(
		struct SqshDirectoryIndexIterator *iterator) {
	(void)iterator;
	return 0;
}

int
sqsh__directory_index_iterator_free(
		struct SqshDirectoryIndexIterator *iterator) {
	if (iterator == NULL) {
		return 0;
	}
	int rv = sqsh__directory_index_iterator_cleanup(iterator);
	free(iterator);
	return rv;
}
