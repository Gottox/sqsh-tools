/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Saturday May 08, 2021 20:14:25 CEST
 */

#include <string.h>

#include "directory.h"
#include "error.h"
#include "inode.h"
#include "squash.h"

static int
directory_data_more(struct SquashDirectory *directory, size_t size) {
	int rv = squash_stream_more(
			&directory->stream, size + sizeof(struct SquashDirectoryHeader));
	if (rv < 0) {
		return rv;
	}

	directory->header = (struct SquashDirectoryHeader *)squash_stream_data(
			&directory->stream);
	directory->entries =
			((void *)directory->header) + sizeof(struct SquashDirectoryHeader);
	return 0;
}

static struct SquashDirectoryEntry *
entry_by_offset(struct SquashDirectoryEntryIterator *iterator, off_t offset) {
	void *tmp = iterator->directory->entries;
	return &tmp[offset];
}

int
squash_directory_init(struct SquashDirectory *directory, struct Squash *squash,
		struct SquashInode *inode) {
	int rv = 0;
	uint32_t dir_block_start;
	uint16_t dir_block_offset;

	switch (inode->wrap->header.inode_type) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		dir_block_start = inode->wrap->data.dir.dir_block_start;
		dir_block_offset = inode->wrap->data.dir.block_offset;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		dir_block_start = inode->wrap->data.xdir.dir_block_start;
		dir_block_offset = inode->wrap->data.xdir.block_offset;
		break;
	default:
		return -SQUASH_ERROR_DIRECTORY_WRONG_INODE_TYPE;
	}

	rv = squash_stream_init(&directory->stream, squash,
			&squash->directory_table, dir_block_start, dir_block_offset);

	// Make sure the header is loaded
	rv = directory_data_more(directory, 0);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

int
squash_directory_count(struct SquashDirectory *directory) {
	return directory->header->count + 1;
}

int
squash_directory_iterator(struct SquashDirectoryEntryIterator *iterator,
		struct SquashDirectory *directory) {
	iterator->count = squash_directory_count(directory);
	iterator->next_offset = 0;
	iterator->directory = directory;

	return 0;
}

int
squash_directory_entry_name_size(struct SquashDirectoryEntry *entry) {
	return entry->name_size + 1;
}

struct SquashDirectoryEntry *
squash_directory_iterator_next(struct SquashDirectoryEntryIterator *iterator) {
	int rv = 0;
	off_t current_offset = iterator->next_offset;

	if (iterator->count == 0) {
		return NULL;
	}

	iterator->count--;
	// Make sure next entry is loaded:
	iterator->next_offset += sizeof(struct SquashDirectoryEntry);
	rv = directory_data_more(iterator->directory, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	// Make sure next entry has its name populated
	iterator->next_offset += squash_directory_entry_name_size(
			entry_by_offset(iterator, current_offset));
	rv = directory_data_more(iterator->directory, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	return entry_by_offset(iterator, current_offset);
}

int
squash_directory_entry_name(
		struct SquashDirectoryEntry *entry, char **name_buffer) {
	int size = squash_directory_entry_name_size(entry);

	char *buffer = calloc(size + 1, sizeof(char));
	if (buffer == NULL) {
		return SQUASH_ERROR_MALLOC_FAILED;
	}
	memcpy(buffer, entry->name, size);

	*name_buffer = buffer;
	return size;
}

int
squash_directory_cleanup(struct SquashDirectory *directory) {
	int rv = 0;
	rv = squash_stream_cleanup(&directory->stream);
	return rv;
}
