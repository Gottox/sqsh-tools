/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include <stdint.h>
#include <stdlib.h>

#include "extract.h"
#include "format/directory.h"

#ifndef DIRECTORY_H

#define DIRECTORY_H

struct SquashInode;

struct SquashDirectory {
	struct SquashInode *inode;
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

int squash_directory_init(struct SquashDirectory *directory,
		struct Squash *squash, struct SquashInode *inode);
const struct SquashDirectoryEntry *squash_directory_lookup(
		struct SquashDirectory *directory, const char *name);
int squash_directory_iterator(struct SquashDirectoryIterator *iterator,
		struct SquashDirectory *directory);
const struct SquashDirectoryEntry *squash_directory_iterator_next(
		struct SquashDirectoryIterator *iterator);

int squash_directory_entry_name_size(const struct SquashDirectoryEntry *entry);
int squash_directory_entry_name(
		const struct SquashDirectoryEntry *entry, char **name_buffer);

int squash_directory_cleanup(struct SquashDirectory *directory);
#endif /* end of include guard DIRECTORY_H */
