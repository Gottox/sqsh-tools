/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Saturday May 08, 2021 20:14:25 CEST
 */

#include "directory_context.h"
#include "../format/directory_internal.h"

#include <string.h>

#include "../error.h"
#include "../format/inode_internal.h"
#include "../format/metablock.h"
#include "../format/superblock.h"
#include "../squash.h"
#include "inode_context.h"
#include "metablock_context.h"

#define METABLOCK_SIZE 8192

int
directory_iterator_index_lookup(
		struct SquashDirectoryIterator *iterator, const char *name) {
	struct SquashInodeContext *inode = iterator->directory->inode;
	if (squash_format_inode_type(inode->inode) !=
			SQUASH_INODE_TYPE_EXTENDED_DIRECTORY) {
		return 0;
	}
	const struct SquashInodeDirectoryExt *xdir =
			squash_format_inode_directory_ext(inode->inode);

	const struct SquashInodeDirectoryIndex *index =
			squash_format_inode_directory_ext_index(xdir);
	size_t index_count = squash_format_inode_directory_ext_index_count(xdir);
	for (int i = 0; i < index_count; i++) {
		const uint8_t *tmp = (uint8_t *)index;
		tmp += sizeof(struct SquashInodeDirectoryIndex) +
				squash_format_inode_directory_index_name_size(index);
	}
	return 0;
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
squash_directory_init(struct SquashDirectoryContext *directory,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode) {
	int rv = 0;

	switch (squash_format_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		directory->block_start = inode->inode->data.directory.block_start;
		directory->block_offset = inode->inode->data.directory.block_offset;
		directory->size = inode->inode->data.directory.file_size - 3;
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		directory->block_start = inode->inode->data.directory_ext.block_start;
		directory->block_offset = inode->inode->data.directory_ext.block_offset;
		directory->size = inode->inode->data.directory_ext.file_size - 3;
		break;
	default:
		return -SQUASH_ERROR_DIRECTORY_WRONG_INODE_TYPE;
	}

	directory->inode = inode;
	directory->superblock = superblock;

	return rv;
}

const struct SquashDirectoryEntry *
squash_directory_lookup(
		struct SquashDirectoryContext *directory, const char *name) {
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
		struct SquashDirectoryContext *directory) {
	int rv = 0;
	const struct SquashMetablock *metablock = squash_metablock_from_offset(
			directory->superblock,
			squash_superblock_directory_table_start(directory->superblock));
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
		iterator->next_offset += sizeof(struct SquashDirectoryFragment);

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
	iterator->next_offset += sizeof(struct SquashDirectoryEntry);
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	struct SquashDirectoryEntry *current =
			entry_by_offset(iterator, current_offset);
	// Make sure next entry has its name populated
	iterator->next_offset += squash_directory_entry_name_size(current);
	rv = directory_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return NULL;
	}

	return current;
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
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
	memcpy(buffer, entry_name, size);

	*name_buffer = buffer;
	return size;
}

int
squash_directory_cleanup(struct SquashDirectoryContext *directory) {
	return 0;
}
