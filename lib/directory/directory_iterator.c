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
 * @file         directory_iterator.c
 */

#include "../../include/sqsh_directory_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode_private.h"
#include "../utils.h"

static int
directory_iterator_index_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;
	struct SqshDirectoryIndexIterator index_iterator = {0};
	struct SqshInode *inode = iterator->inode;
	const uint64_t inode_ref = sqsh_inode_ref(inode);

	rv = sqsh__directory_index_iterator_init(
			&index_iterator, inode->archive, inode_ref);
	if (rv < 0) {
		return rv;
	}
	while ((rv = sqsh__directory_index_iterator_next(&index_iterator)) > 0) {
		const char *index_name =
				sqsh__directory_index_iterator_name(&index_iterator);
		const uint32_t index_name_size =
				sqsh__directory_index_iterator_name_size(&index_iterator);

		// BUG: the branch could be taken too early when the name is a prefix
		if (strncmp(name, (char *)index_name,
					SQSH_MIN(index_name_size, name_len + 1)) < 0) {
			break;
		}

		const uint32_t index_offset =
				sqsh__directory_index_iterator_index(&index_iterator);
		if (SQSH_ADD_OVERFLOW(
					iterator->next_offset, index_offset,
					&iterator->next_offset)) {
			return -SQSH_ERROR_INTEGER_OVERFLOW;
		}
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
	const uint8_t *data = sqsh__metablock_reader_data(&iterator->metablock);
	return (struct SqshDataDirectoryEntry *)data;
}

static struct SqshDataDirectoryFragment *
get_fragment(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *data = sqsh__metablock_reader_data(&iterator->metablock);
	return (struct SqshDataDirectoryFragment *)data;
}

int
sqsh_directory_iterator_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;

	if (sqsh_inode_is_extended(iterator->inode)) {
		rv = directory_iterator_index_lookup(iterator, name, name_len);
		if (rv < 0) {
			return rv;
		}
	}

	while (sqsh_directory_iterator_next(iterator) > 0) {
		const size_t entry_name_size =
				sqsh_directory_iterator_name_size(iterator);
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
		struct SqshDirectoryIterator *iterator, struct SqshInode *inode) {
	int rv = 0;
	struct SqshArchive *archive = inode->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

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

	if (SQSH_SUB_OVERFLOW(
				sqsh_inode_file_size(inode), 3, &iterator->remaining_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	rv = sqsh__metablock_reader_init(
			&iterator->metablock, archive, NULL, start_address, upper_limit);
	if (rv < 0) {
		return rv;
	}

	iterator->next_offset = inner_offset;
	iterator->inode = inode;

	return rv;
}

struct SqshDirectoryIterator *
sqsh_directory_iterator_new(struct SqshInode *inode, int *err) {
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
	return sqsh__data_directory_entry_name_size(entry) + 1;
}

uint64_t
sqsh_directory_iterator_inode_ref(
		const struct SqshDirectoryIterator *iterator) {
	const uint32_t block_index = iterator->start_base;
	const uint16_t block_offset =
			sqsh__data_directory_entry_offset(get_entry(iterator));

	return sqsh_address_ref_create(block_index, block_offset);
}

uint64_t
sqsh_directory_iterator_inode_number(
		const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	const uint32_t inode_base = iterator->inode_base;
	const uint16_t inode_offset =
			sqsh__data_directory_entry_inode_offset(entry);

	return inode_base + inode_offset;
}

enum SqshInodeType
sqsh_directory_iterator_inode_type(
		const struct SqshDirectoryIterator *iterator) {
	switch (sqsh__data_directory_entry_type(get_entry(iterator))) {
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

struct SqshInode *
sqsh_directory_iterator_inode_load(
		const struct SqshDirectoryIterator *iterator, int *err) {
	const uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);
	struct SqshArchive *archive = iterator->inode->archive;

	return sqsh_inode_new(archive, inode_ref, err);
}

static int
process_fragment(struct SqshDirectoryIterator *iterator) {
	int rv = 0;

	rv = sqsh__metablock_reader_advance(
			&iterator->metablock, iterator->next_offset,
			SQSH_SIZEOF_DIRECTORY_FRAGMENT);
	if (rv < 0) {
		return rv;
	}

	const struct SqshDataDirectoryFragment *fragment = get_fragment(iterator);
	iterator->remaining_entries =
			sqsh__data_directory_fragment_count(fragment) + 1;
	iterator->start_base = sqsh__data_directory_fragment_start(fragment);
	iterator->inode_base = sqsh__data_directory_fragment_inode_number(fragment);

	iterator->next_offset = SQSH_SIZEOF_DIRECTORY_FRAGMENT;

	if (SQSH_SUB_OVERFLOW(
				iterator->remaining_size, SQSH_SIZEOF_DIRECTORY_FRAGMENT,
				&iterator->remaining_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	return rv;
}

int
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator) {
	int rv = 0;
	size_t size;

	if (iterator->remaining_size == 0) {
		return 0;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		rv = process_fragment(iterator);
		if (rv < 0) {
			return rv;
		}
	}

	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	size = SQSH_SIZEOF_DIRECTORY_ENTRY;
	rv = sqsh__metablock_reader_advance(
			&iterator->metablock, iterator->next_offset, size);
	if (rv < 0) {
		return rv;
	}

	// Make sure next entry has its name populated
	if (SQSH_ADD_OVERFLOW(
				size, sqsh_directory_iterator_name_size(iterator), &size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	// May invalidate pointers into directory entries. that's why the
	// get_entry() call is repeated below.
	rv = sqsh__metablock_reader_advance(&iterator->metablock, 0, size);
	if (rv < 0) {
		return rv;
	}

	iterator->next_offset = size;
	if (SQSH_SUB_OVERFLOW(
				iterator->remaining_size, size, &iterator->remaining_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	return 1;
}

const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	return (char *)sqsh__data_directory_entry_name(entry);
}

char *
sqsh_directory_iterator_name_dup(const struct SqshDirectoryIterator *iterator) {
	int size = sqsh_directory_iterator_name_size(iterator);
	const char *entry_name = sqsh_directory_iterator_name(iterator);

	return sqsh_memdup(entry_name, size);
}

int
sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator) {
	int rv = 0;
	rv = sqsh__metablock_reader_cleanup(&iterator->metablock);
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
