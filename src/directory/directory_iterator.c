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

#include <string.h>

static int
directory_iterator_index_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;
	struct SqshDirectoryIndexIterator index_iterator = {0};
	struct SqshInodeContext *inode = iterator->inode;

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

static const struct SqshDataDirectoryFragment *
directory_iterator_current_fragment(
		const struct SqshDirectoryIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (const struct SqshDataDirectoryFragment
					*)&tmp[iterator->current_fragment_offset];
}

static int
directory_data_more(struct SqshDirectoryIterator *iterator, size_t size) {
	int rv = sqsh__metablock_stream_more(&iterator->metablock, size);
	if (rv < 0) {
		return rv;
	}

	iterator->fragments =
			(struct SqshDataDirectoryFragment *)sqsh__metablock_stream_data(
					&iterator->metablock);
	return 0;
}

static struct SqshDataDirectoryEntry *
current_entry(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (struct SqshDataDirectoryEntry *)&tmp[iterator->current_offset];
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
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_DIRECTORY) {
		return -SQSH_ERROR_NOT_A_DIRECTORY;
	}
	iterator->block_start = sqsh_inode_directory_block_start(inode);
	iterator->block_offset = sqsh_inode_directory_block_offset(inode);
	iterator->size = sqsh_inode_file_size(inode) - 3;
	iterator->inode = inode;

	rv = sqsh__metablock_stream_init(
			&iterator->metablock, sqsh,
			sqsh_superblock_directory_table_start(superblock), ~0);
	if (rv < 0) {
		return rv;
	}
	rv = sqsh__metablock_stream_seek(
			&iterator->metablock, iterator->block_start,
			iterator->block_offset);
	if (rv < 0) {
		return rv;
	}

	iterator->current_fragment_offset = 0;
	iterator->remaining_entries = 0;
	iterator->next_offset = 0;
	iterator->current_offset = 0;

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
	const struct SqshDataDirectoryEntry *entry = current_entry(iterator);
	return sqsh_data_directory_entry_name_size(entry) + 1;
}

uint64_t
sqsh_directory_iterator_inode_ref(
		const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryFragment *fragment =
			directory_iterator_current_fragment(iterator);
	uint32_t block_index = sqsh_data_directory_fragment_start(fragment);
	uint16_t block_offset =
			sqsh_data_directory_entry_offset(current_entry(iterator));

	return sqsh_inode_ref_from_block(block_index, block_offset);
}

enum SqshInodeContextType
sqsh_directory_iterator_inode_type(
		const struct SqshDirectoryIterator *iterator) {
	switch (sqsh_data_directory_entry_type(current_entry(iterator))) {
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
	iterator->current_offset = iterator->next_offset;

	if (iterator->next_offset >= iterator->size) {
		// TODO: Check if 0 is really okay here.
		return 0;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		iterator->next_offset += SQSH_SIZEOF_DIRECTORY_FRAGMENT;

		rv = directory_data_more(iterator, iterator->next_offset);
		if (rv < 0) {
			return rv;
		}
		iterator->current_fragment_offset = iterator->current_offset;

		const struct SqshDataDirectoryFragment *current_fragment =
				directory_iterator_current_fragment(iterator);
		iterator->remaining_entries =
				sqsh_data_directory_fragment_count(current_fragment) + 1;

		iterator->current_offset = iterator->next_offset;
	}
	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += SQSH_SIZEOF_DIRECTORY_ENTRY;
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure next entry has its name populated
	iterator->next_offset += sqsh_directory_iterator_name_size(iterator);
	// May invalidate pointers into directory entries. that's why the
	// current_entry() call is repeated below.
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return 1;
}

const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = current_entry(iterator);
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
	rv = sqsh__metablock_stream_cleanup(&iterator->metablock);
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