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
 * @file         directory.c
 */

#include "../../include/sqsh.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_directory_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode_private.h"
#include "../utils.h"

#include <stdint.h>
#include <string.h>

static int
directory_iterator_index_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;
	struct SqshDirectoryIndexIterator index_iterator = {0};
	struct SqshInodeContext *inode = iterator->inode;

	// disable fast index lookup for now.
	return 0;

	rv = sqsh__directory_index_iterator_init(&index_iterator, inode);
	if (rv < 0) {
		return 0;
	}
	while ((rv = sqsh__directory_index_iterator_next(&index_iterator)) > 0) {
		const char *index_name =
				sqsh__directory_index_iterator_name(&index_iterator);
		uint32_t index_name_size =
				sqsh__directory_index_iterator_name_size(&index_iterator);

		// BUG: the branch could be taken too early when the name is a prefix
		if (strncmp(name, (char *)index_name,
					SQSH_MIN(index_name_size, name_len + 1)) < 0) {
			break;
		}
		iterator->next_offset =
				sqsh__directory_index_iterator_index(&index_iterator);
	}
	iterator->remaining_entries = 0;
	if (rv < 0) {
		return rv;
	}
	rv = sqsh__directory_index_iterator_cleanup(&index_iterator);
	return rv;
}

static struct SqshDataDirectoryEntry *
get_entry(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *data = sqsh__metablock_cursor_data(&iterator->metablock);
	return (struct SqshDataDirectoryEntry *)data;
}

static struct SqshDataDirectoryFragment *
get_fragment(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *data = sqsh__metablock_cursor_data(&iterator->metablock);
	return (struct SqshDataDirectoryFragment *)data;
}

int
sqsh_directory_iterator_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;

	rv = directory_iterator_index_lookup(iterator, name, name_len);
	if (rv < 0)
		return rv;

	while (sqsh_directory_iterator_next(iterator) > 0) {
		size_t entry_name_size = sqsh_directory_iterator_name_size(iterator);
		const char *entry_name = sqsh_directory_iterator_name(iterator);
		if (name_len != entry_name_size) {
			continue;
		}
		if (strncmp(name, (char *)entry_name, entry_name_size) == 0) {
			return 0;
		}
	}

	return -SQSH_ERROR_NO_SUCH_FILE;
}

int
sqsh__directory_iterator_init(
		struct SqshDirectoryIterator *iterator,
		struct SqshInodeContext *inode) {
	int rv = 0;
	struct Sqsh *sqsh = inode->sqsh;
	const struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_DIRECTORY) {
		return -SQSH_ERROR_NOT_A_DIRECTORY;
	}

	const uint64_t outer_offset = sqsh_inode_directory_block_start(inode);
	const uint32_t inner_offset = sqsh_inode_directory_block_offset(inode);
	uint64_t start_address = sqsh_superblock_directory_table_start(superblock);
	// TODO: Use a better upper limit
	const uint64_t upper_limit = sqsh_superblock_bytes_used(superblock);
	if (SQSH_ADD_OVERFLOW(start_address, outer_offset, &start_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	iterator->size = sqsh_inode_file_size(inode) - 3;
	iterator->inode = inode;

	rv = sqsh__metablock_cursor_init(
			&iterator->metablock, sqsh, start_address, upper_limit);
	if (rv < 0) {
		return rv;
	}

	iterator->next_offset = inner_offset;

	return rv;
}

struct SqshDirectoryIterator *
sqsh_directory_iterator_new(struct SqshInodeContext *inode, int *err) {
	struct SqshDirectoryIterator *iterator =
			calloc(1, sizeof(struct SqshDirectoryIterator));
	if (iterator == NULL) {
		return NULL;
	}
	*err = sqsh__directory_iterator_init(iterator, inode);
	if (*err < 0) {
		free(iterator);
		return NULL;
	}
	return iterator;
}

int
sqsh_directory_iterator_name_size(
		const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	return sqsh_data_directory_entry_name_size(entry) + 1;
}

uint64_t
sqsh_directory_iterator_inode_ref(
		const struct SqshDirectoryIterator *iterator) {
	uint32_t block_index = iterator->start_base;
	uint16_t block_offset =
			sqsh_data_directory_entry_offset(get_entry(iterator));

	return sqsh_inode_ref_from_block(block_index, block_offset);
}

enum SqshInodeContextType
sqsh_directory_iterator_inode_type(
		const struct SqshDirectoryIterator *iterator) {
	switch (sqsh_data_directory_entry_type(get_entry(iterator))) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		return SQSH_INODE_TYPE_DIRECTORY;
	case SQSH_INODE_TYPE_BASIC_FILE:
		return SQSH_INODE_TYPE_FILE;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		return SQSH_INODE_TYPE_SYMLINK;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
		return SQSH_INODE_TYPE_BLOCK;
	case SQSH_INODE_TYPE_BASIC_CHAR:
		return SQSH_INODE_TYPE_CHAR;
	case SQSH_INODE_TYPE_BASIC_FIFO:
		return SQSH_INODE_TYPE_FIFO;
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		return SQSH_INODE_TYPE_SOCKET;
	}
	return SQSH_INODE_TYPE_UNKNOWN;
}

struct SqshInodeContext *
sqsh_directory_iterator_inode_load(
		const struct SqshDirectoryIterator *iterator, int *err) {
	uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);
	struct Sqsh *sqsh = iterator->inode->sqsh;

	return sqsh_inode_new(sqsh, inode_ref, err);
}

int
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator) {
	int rv = 0;
	uint64_t next_offset = iterator->next_offset;
	size_t size = 0;
	if (iterator->size == 0) {
		// TODO: Check if 0 is really okay here.
		return 0;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		rv = sqsh__metablock_cursor_advance(
				&iterator->metablock, next_offset,
				SQSH_SIZEOF_DIRECTORY_FRAGMENT);
		if (rv < 0) {
			return rv;
		}

		const struct SqshDataDirectoryFragment *fragment =
				get_fragment(iterator);
		iterator->remaining_entries =
				sqsh_data_directory_fragment_count(fragment) + 1;
		iterator->start_base = sqsh_data_directory_fragment_start(fragment);
		iterator->inode_base =
				sqsh_data_directory_fragment_inode_number(fragment);

		next_offset = SQSH_SIZEOF_DIRECTORY_FRAGMENT;
		iterator->size -= SQSH_SIZEOF_DIRECTORY_FRAGMENT;
	}
	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	size = SQSH_SIZEOF_DIRECTORY_ENTRY;
	rv = sqsh__metablock_cursor_advance(
			&iterator->metablock, next_offset, size);
	if (rv < 0) {
		return rv;
	}

	// Make sure next entry has its name populated
	if (SQSH_ADD_OVERFLOW(
				size, sqsh_directory_iterator_name_size(iterator), &size)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	// May invalidate pointers into directory entries. that's why the
	// get_entry() call is repeated below.
	rv = sqsh__metablock_cursor_advance(&iterator->metablock, 0, size);
	if (rv < 0) {
		return rv;
	}

	iterator->next_offset = size;
	iterator->size -= size;
	return 1;
}

const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	return (char *)sqsh_data_directory_entry_name(entry);
}

int
sqsh_directory_iterator_name_dup(
		const struct SqshDirectoryIterator *iterator, char **name_buffer) {
	int size = sqsh_directory_iterator_name_size(iterator);
	const char *entry_name = sqsh_directory_iterator_name(iterator);

	*name_buffer = sqsh_memdup(entry_name, size);
	if (*name_buffer) {
		return size;
	} else {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
}

int
sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator) {
	int rv = 0;
	rv = sqsh__metablock_cursor_cleanup(&iterator->metablock);
	return rv;
}

int
sqsh_directory_iterator_free(struct SqshDirectoryIterator *iterator) {
	if (iterator == NULL) {
		return 0;
	}
	int rv = sqsh__directory_iterator_cleanup(iterator);
	free(iterator);
	return rv;
}
