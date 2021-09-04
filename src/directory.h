/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include <stdint.h>

#include "stream.h"

#ifndef DIRECTORY_H

#define DIRECTORY_H

struct SquashDirectoryHeader {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
};

struct SquashDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	uint8_t name[0]; // [name_size + 1]
};

struct SquashDirectory {
	struct SquashDirectoryHeader *header;
	struct SquashDirectoryEntry *entries;
	struct SquashStream stream;
};

struct SquashDirectoryEntryIterator {
	struct SquashDirectory *directory;
	size_t count;
	off_t next_offset;
};

struct SquashInode;

int squash_directory_init(struct SquashDirectory *directory,
		struct Squash *squash, struct SquashInode *inode);
int squash_directory_count(struct SquashDirectory *directory);
int squash_directory_iterator(struct SquashDirectoryEntryIterator *iterator,
		struct SquashDirectory *directory);
struct SquashDirectoryEntry *squash_directory_iterator_next(
		struct SquashDirectoryEntryIterator *iterator);

int squash_directory_entry_name_size(struct SquashDirectoryEntry *entry);
int squash_directory_entry_name(
		struct SquashDirectoryEntry *entry, char **name_buffer);

int squash_directory_cleanup(struct SquashDirectory *directory);
#endif /* end of include guard DIRECTORY_H */
