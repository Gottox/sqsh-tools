/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Saturday May 08, 2021 20:14:25 CEST
 */

#include <stdint.h>
#include <string.h>

#include "directory.h"
#include "error.h"
#include "format/superblock.h"
#include "inode.h"
#include "squash.h"

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

static const struct SquashDirectoryFragment *
fragment_by_offset(struct SquashDirectoryIterator *iterator, off_t offset) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (const struct SquashDirectoryFragment *)&tmp[offset];
}

static struct SquashDirectoryEntry *
entry_by_offset(struct SquashDirectoryIterator *iterator, off_t offset) {
	const uint8_t *tmp = (const uint8_t *)iterator->fragments;
	return (struct SquashDirectoryEntry *)&tmp[offset];
}

int
squash_directory_init(struct SquashDirectory *directory, struct Squash *squash,
		struct SquashInode *inode) {
	int rv = 0;

	switch (inode->wrap->header.inode_type) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		directory->block_start = inode->wrap->data.dir.dir_block_start;
		directory->block_offset = inode->wrap->data.dir.block_offset;
		directory->size = inode->wrap->data.dir.file_size - 3;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		directory->block_start = inode->wrap->data.xdir.dir_block_start;
		directory->block_offset = inode->wrap->data.xdir.block_offset;
		directory->size = inode->wrap->data.xdir.file_size - 3;
		break;
	default:
		return -SQUASH_ERROR_DIRECTORY_WRONG_INODE_TYPE;
	}

	directory->inode = inode;
	directory->squash = squash;

	return rv;
}

const struct SquashDirectoryEntry *
squash_directory_lookup(struct SquashDirectory *directory, const char *name) {
	int rv = 0;
	struct SquashDirectoryIterator iterator = {0};
	const struct SquashDirectoryEntry *entry;
	const size_t name_len = strlen(name);

	rv = squash_directory_iterator_init(&iterator, directory);
	if (rv < 0)
		return NULL;

	while ((entry = squash_directory_iterator_next(&iterator))) {
		size_t entry_name_size = squash_directory_entry_name_size(entry);
		const uint8_t *entry_name = squash_format_directory_entry_name(entry);
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
		struct SquashDirectory *directory) {
	int rv = 0;
	const struct SquashMetablock *metablock =
			squash_metablock_from_offset(directory->squash,
					squash_superblock_directory_table_start(
							directory->squash->superblock));
	if (metablock == NULL) {
		return -SQUASH_ERROR_DIRECTORY_INIT;
	}
	rv = squash_extract_init(&iterator->extract, directory->squash, metablock,
			directory->block_start, directory->block_offset);
	if (rv < 0) {
		return -SQUASH_ERROR_DIRECTORY_INIT;
	}

	iterator->directory = directory;
	iterator->current_fragment_offset = 0;
	iterator->remaining_entries = 0;
	iterator->next_offset = 0;

	return rv;
}

int
squash_directory_entry_name_size(const struct SquashDirectoryEntry *entry) {
	return squash_format_directory_entry_name_size(entry) + 1;
}

const struct SquashDirectoryEntry *
squash_directory_iterator_next(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	off_t current_offset = iterator->next_offset;

	if (current_offset >= iterator->directory->size) {
		return NULL;
	} else if (iterator->remaining_entries == 0) {
		// New fragment begins
		iterator->next_offset += SQUASH_DIRECTORY_FRAGMENT_SIZE;

		rv = directory_data_more(iterator, iterator->next_offset);
		if (rv < 0) {
			return NULL;
		}
		iterator->current_fragment_offset = current_offset;

		const struct SquashDirectoryFragment *current_fragment =
				fragment_by_offset(iterator, iterator->current_fragment_offset);
		current_offset = iterator->next_offset;
		iterator->remaining_entries =
				squash_format_directory_fragment_count(current_fragment) + 1;
	}
	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += SQUASH_DIRECTORY_ENTRY_SIZE;
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	// Make sure next entry has its name populated
	iterator->next_offset += squash_directory_entry_name_size(
			entry_by_offset(iterator, current_offset));
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	return entry_by_offset(iterator, current_offset);
}

int
squash_directory_iterator_clean(struct SquashDirectoryIterator *iterator) {
	int rv = 0;
	rv = squash_extract_cleanup(&iterator->extract);
	return rv;
}

int
squash_directory_entry_name(
		const struct SquashDirectoryEntry *entry, char **name_buffer) {
	int size = squash_directory_entry_name_size(entry);
	const uint8_t *entry_name = squash_format_directory_entry_name(entry);

	char *buffer = calloc(size + 1, sizeof(char));
	if (buffer == NULL) {
		return SQUASH_ERROR_MALLOC_FAILED;
	}
	memcpy(buffer, entry_name, size);

	*name_buffer = buffer;
	return size;
}

int
squash_directory_cleanup(struct SquashDirectory *directory) {
	return 0;
}
