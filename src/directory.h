/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include <stddef.h>
#include <stdint.h>
#include "extract.h"

#ifndef DIRECTORY_H

#define DIRECTORY_H

struct SquashDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	uint8_t name[0]; // [name_size + 1]
};

struct SquashDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	struct SquashDirectoryEntry entries[0]; // [count + 1]
};

struct SquashDirectory {
	struct SquashDirectoryFragment *fragments;
	struct SquashExtract extract;
	uint32_t size;
};

struct SquashDirectoryIterator {
	struct SquashDirectory *directory;
	off_t current_fragment_offset;
	size_t remaining_entries;
	off_t next_offset;
};

struct SquashInode;

int squash_directory_init(struct SquashDirectory *directory,
		struct Squash *squash, struct SquashInode *inode);
int squash_directory_count(struct SquashDirectory *directory);
int squash_directory_iterator(struct SquashDirectoryIterator *iterator,
		struct SquashDirectory *directory);
struct SquashDirectoryEntry *squash_directory_iterator_next(
		struct SquashDirectoryIterator *iterator);

int squash_directory_entry_name_size(struct SquashDirectoryEntry *entry);
int squash_directory_entry_name(
		struct SquashDirectoryEntry *entry, char **name_buffer);

int squash_directory_cleanup(struct SquashDirectory *directory);
#endif /* end of include guard DIRECTORY_H */
