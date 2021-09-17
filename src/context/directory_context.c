/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Saturday May 08, 2021 20:14:25 CEST
 */

#include "directory_context.h"
#include "../data/directory.h"

#include <stdint.h>
#include <string.h>

#include "../data/inode.h"
#include "../data/metablock.h"
#include "../data/superblock.h"
#include "../error.h"
#include "../squash.h"
#include "inode_context.h"
#include "metablock_context.h"

#define METABLOCK_SIZE 8192

static int
directory_iterator_index_lookup(struct SquashDirectoryIterator *iterator,
		const char *name, const size_t name_len) {
	int rv = 0;
	struct SquashInodeDirectoryIndexIterator index_iterator;
	const struct SquashInodeDirectoryIndex *index;
	const struct SquashInodeDirectoryIndex *candidate = NULL;
	struct SquashInodeContext *inode = iterator->directory->inode;
	if (squash_data_inode_type(inode->inode) !=
			SQUASH_INODE_TYPE_EXTENDED_DIRECTORY) {
		return 0;
	}

	rv = squash_inode_directory_iterator_init(&index_iterator, inode);
	if (rv < 0) {
		return 0;
	}
	while ((index = squash_inode_directory_index_iterator_next(
					&index_iterator))) {
		const uint8_t *index_name =
				squash_data_inode_directory_index_name(index);
		uint32_t index_name_size =
				squash_data_inode_directory_index_name_size(index) + 1;

		if (strncmp(name, (char *)index_name, MIN(index_name_size, name_len)) >
				0) {
			break;
		}
		candidate = index;
	}
	if (candidate) {
		iterator->remaining_entries = 0;
		// TODO: do not decompress everything for the lookup.
		iterator->next_offset =
				squash_data_inode_directory_index_start(candidate);
	}
	return rv;
}

static int
directory_data_more(struct SquashDirectoryIterator *iterator, size_t size) {
	int rv = squash_extract_more(&iterator->extract, size);
	if (rv < 0) {
		return rv;
	}

	iterator->fragments = (struct SquashDirectoryFragment *)squash_extract_data(
			&iterator->extract);
	return 0;
}

static struct SquashDirectoryEntry *
entry_by_offset(struct SquashDirectoryIterator *iterator, off_t offset) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (struct SquashDirectoryEntry *)&tmp[offset];
}

int
squash_directory_init(struct SquashDirectoryContext *directory,
		const struct SquashSuperblock *superblock,
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
		return -SQUASH_ERROR_DIRECTORY_WRONG_INODE_TYPE;
	}

	directory->inode = inode;
	directory->superblock = superblock;

	return rv;
}

const struct SquashDirectoryEntry *
squash_directory_iterator_lookup(struct SquashDirectoryIterator *iterator,
		const char *name, const size_t name_len) {
	int rv = 0;
	const struct SquashDirectoryEntry *entry;

	rv = directory_iterator_index_lookup(iterator, name, name_len);
	if (rv < 0)
		return NULL;

	while ((entry = squash_directory_iterator_next(iterator))) {
		size_t entry_name_size = squash_directory_entry_name_size(entry);
		const uint8_t *entry_name = squash_data_directory_entry_name(entry);
		if (name_len != entry_name_size) {
			continue;
		}
		if (strncmp(name, (char *)entry_name, entry_name_size) == 0) {
			return entry;
		}
	}

	return NULL;
}

int
squash_directory_iterator_init(struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory) {
	int rv = 0;
	const struct SquashMetablock *metablock =
			squash_metablock_from_offset(directory->superblock,
					squash_data_superblock_directory_table_start(
							directory->superblock));
	if (metablock == NULL) {
		return -SQUASH_ERROR_DIRECTORY_INIT;
	}
	rv = squash_extract_init(&iterator->extract, directory->superblock,
			metablock, directory->block_start, directory->block_offset);
	if (rv < 0) {
		return rv;
	}

	iterator->directory = directory;
	iterator->current_fragment_offset = 0;
	iterator->remaining_entries = 0;
	iterator->next_offset = 0;

	return rv;
}

int
squash_directory_entry_name_size(const struct SquashDirectoryEntry *entry) {
	return squash_data_directory_entry_name_size(entry) + 1;
}

const struct SquashDirectoryFragment *
squash_directory_iterator_current_fragment(
		const struct SquashDirectoryIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (const struct SquashDirectoryFragment
					*)&tmp[iterator->current_fragment_offset];
}

const struct SquashDirectoryEntry *
squash_directory_iterator_next(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	off_t current_offset = iterator->next_offset;

	if (iterator->next_offset >= iterator->directory->size) {
		return NULL;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		iterator->next_offset += SQUASH_SIZEOF_DIRECTORY_FRAGMENT;

		rv = directory_data_more(iterator, iterator->next_offset);
		if (rv < 0) {
			return NULL;
		}
		iterator->current_fragment_offset = current_offset;

		const struct SquashDirectoryFragment *current_fragment =
				squash_directory_iterator_current_fragment(iterator);
		iterator->remaining_entries =
				squash_data_directory_fragment_count(current_fragment) + 1;

		current_offset = iterator->next_offset;
	}
	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += SQUASH_SIZEOF_DIRECTORY_ENTRY;
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	// Make sure next entry has its name populated
	iterator->next_offset += squash_directory_entry_name_size(
			entry_by_offset(iterator, current_offset));
	// May invalidate pointers into directory entries. that's why the
	// entry_by_offset() call isn't safed to a pointer and reused later.
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	return entry_by_offset(iterator, current_offset);
}

int
squash_directory_iterator_cleanup(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	rv = squash_extract_cleanup(&iterator->extract);
	return rv;
}

const char *
squash_directory_entry_name(const struct SquashDirectoryEntry *entry) {
	return (char *)squash_data_directory_entry_name(entry);
}

int
squash_directory_entry_name_dup(
		const struct SquashDirectoryEntry *entry, char **name_buffer) {
	int size = squash_directory_entry_name_size(entry);
	const char *entry_name = squash_directory_entry_name(entry);

	return squash_memdup(name_buffer, entry_name, size);
}

int
squash_directory_cleanup(struct SquashDirectoryContext *directory) {
	return 0;
}
