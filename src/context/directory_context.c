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
 * @file        : directory
 * @created     : Saturday May 08, 2021 20:14:25 CEST
 */

#include "directory_context.h"
#include "../data/directory.h"
#include "../data/inode.h"
#include "../data/metablock.h"
#include "../error.h"
#include "../squash.h"
#include "inode_context.h"
#include "metablock_context.h"

#include <string.h>

static int
directory_iterator_index_lookup(
		struct SquashDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;
	struct SquashInodeDirectoryIndexIterator index_iterator;
	struct SquashInodeContext *inode = iterator->directory->inode;

	rv = squash_inode_directory_iterator_init(&index_iterator, inode);
	if (rv < 0) {
		return 0;
	}
	while (squash_inode_directory_index_iterator_next(&index_iterator)) {
		const char *index_name =
				squash_inode_directory_index_iterator_name(&index_iterator);
		uint32_t index_name_size =
				squash_inode_directory_index_iterator_name_size(
						&index_iterator);

		if (strncmp(name, (char *)index_name, MIN(index_name_size, name_len)) >
			0) {
			break;
		}
		iterator->next_offset =
				squash_inode_directory_index_iterator_index(&index_iterator);
	}
	iterator->remaining_entries = 0;
	return rv;
}

static const struct SquashDirectoryFragment *
directory_iterator_current_fragment(
		const struct SquashDirectoryIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (const struct SquashDirectoryFragment
					*)&tmp[iterator->current_fragment_offset];
}

static int
directory_data_more(struct SquashDirectoryIterator *iterator, size_t size) {
	int rv = squash_metablock_more(&iterator->extract, size);
	if (rv < 0) {
		return rv;
	}

	iterator->fragments =
			(struct SquashDirectoryFragment *)squash_metablock_data(
					&iterator->extract);
	return 0;
}

static struct SquashDirectoryEntry *
current_entry(const struct SquashDirectoryIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (struct SquashDirectoryEntry *)&tmp[iterator->current_offset];
}

int
squash_directory_init(
		struct SquashDirectoryContext *directory,
		const struct SquashSuperblockContext *superblock,
		struct SquashInodeContext *inode) {
	int rv = 0;
	const struct SquashInodeDirectory *basic;
	const struct SquashInodeDirectoryExt *extended;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		basic = squash_data_inode_directory(inode->inode);
		directory->block_start = squash_data_inode_directory_block_start(basic);
		directory->block_offset =
				squash_data_inode_directory_block_offset(basic);
		directory->size = squash_data_inode_directory_file_size(basic) - 3;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended = squash_data_inode_directory_ext(inode->inode);
		directory->block_start =
				squash_data_inode_directory_ext_block_start(extended);
		directory->block_offset =
				squash_data_inode_directory_ext_block_offset(extended);
		directory->size =
				squash_data_inode_directory_ext_file_size(extended) - 3;
		break;
	default:
		return -SQUASH_ERROR_NOT_A_DIRECTORY;
	}

	directory->inode = inode;
	directory->superblock = superblock;

	return rv;
}

int
squash_directory_iterator_lookup(
		struct SquashDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;

	rv = directory_iterator_index_lookup(iterator, name, name_len);
	if (rv < 0)
		return rv;

	while (squash_directory_iterator_next(iterator) > 0) {
		size_t entry_name_size = squash_directory_iterator_name_size(iterator);
		const char *entry_name = squash_directory_iterator_name(iterator);
		if (name_len != entry_name_size) {
			continue;
		}
		if (strncmp(name, (char *)entry_name, entry_name_size) == 0) {
			return 0;
		}
	}

	return -SQUASH_ERROR_NO_SUCH_FILE;
}

int
squash_directory_iterator_init(
		struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory) {
	int rv = 0;
	rv = squash_metablock_init(
			&iterator->extract, directory->superblock,
			squash_superblock_directory_table_start(directory->superblock));
	if (rv < 0) {
		return rv;
	}
	rv = squash_metablock_seek(
			&iterator->extract, directory->block_start,
			directory->block_offset);
	if (rv < 0) {
		return rv;
	}

	iterator->directory = directory;
	iterator->current_fragment_offset = 0;
	iterator->remaining_entries = 0;
	iterator->next_offset = 0;
	iterator->current_offset = 0;

	return rv;
}

int
squash_directory_iterator_name_size(
		const struct SquashDirectoryIterator *iterator) {
	const struct SquashDirectoryEntry *entry = current_entry(iterator);
	return squash_data_directory_entry_name_size(entry) + 1;
}

uint64_t
squash_directory_iterator_inode_ref(
		const struct SquashDirectoryIterator *iterator) {
	const struct SquashDirectoryFragment *fragment =
			directory_iterator_current_fragment(iterator);
	uint32_t block_index = squash_data_directory_fragment_start(fragment);
	uint16_t block_offset =
			squash_data_directory_entry_offset(current_entry(iterator));

	return squash_inode_ref_from_block(block_index, block_offset);
}

enum SquashInodeContextType
squash_directory_iterator_inode_type(
		const struct SquashDirectoryIterator *iterator) {
	switch (squash_data_directory_entry_type(current_entry(iterator))) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		return SQUASH_INODE_TYPE_DIRECTORY;
	case SQUASH_INODE_TYPE_BASIC_FILE:
		return SQUASH_INODE_TYPE_FILE;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		return SQUASH_INODE_TYPE_SYMLINK;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
		return SQUASH_INODE_TYPE_BLOCK;
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		return SQUASH_INODE_TYPE_CHAR;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
		return SQUASH_INODE_TYPE_FIFO;
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		return SQUASH_INODE_TYPE_SOCKET;
	}
	return SQUASH_INODE_TYPE_UNKNOWN;
}

int
squash_directory_iterator_inode_load(
		const struct SquashDirectoryIterator *iterator,
		struct SquashInodeContext *inode) {
	uint64_t inode_ref = squash_directory_iterator_inode_ref(iterator);

	return squash_inode_load(inode, iterator->directory->superblock, inode_ref);
}

int
squash_directory_iterator_next(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	iterator->current_offset = iterator->next_offset;

	if (iterator->next_offset >= iterator->directory->size) {
		// TODO: Check if 0 is really okay here.
		return 0;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		iterator->next_offset += SQUASH_SIZEOF_DIRECTORY_FRAGMENT;

		rv = directory_data_more(iterator, iterator->next_offset);
		if (rv < 0) {
			return rv;
		}
		iterator->current_fragment_offset = iterator->current_offset;

		const struct SquashDirectoryFragment *current_fragment =
				directory_iterator_current_fragment(iterator);
		iterator->remaining_entries =
				squash_data_directory_fragment_count(current_fragment) + 1;

		iterator->current_offset = iterator->next_offset;
	}
	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += SQUASH_SIZEOF_DIRECTORY_ENTRY;
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure next entry has its name populated
	iterator->next_offset += squash_directory_iterator_name_size(iterator);
	// May invalidate pointers into directory entries. that's why the
	// current_entry() call is repeated below.
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return 1;
}

int
squash_directory_iterator_cleanup(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	rv = squash_metablock_cleanup(&iterator->extract);
	return rv;
}

const char *
squash_directory_iterator_name(const struct SquashDirectoryIterator *iterator) {
	const struct SquashDirectoryEntry *entry = current_entry(iterator);
	return (char *)squash_data_directory_entry_name(entry);
}

int
squash_directory_iterator_name_dup(
		const struct SquashDirectoryIterator *iterator, char **name_buffer) {
	int size = squash_directory_iterator_name_size(iterator);
	const char *entry_name = squash_directory_iterator_name(iterator);

	*name_buffer = squash_memdup(entry_name, size);
	if (*name_buffer) {
		return size;
	} else {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
}

int
squash_directory_cleanup(struct SquashDirectoryContext *directory) {
	return 0;
}
