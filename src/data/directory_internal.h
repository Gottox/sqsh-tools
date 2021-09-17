/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory_internal
 * @created     : Wednesday Sep 08, 2021 16:44:48 CEST
 */

#include "directory.h"

#ifndef DIRECTORY_INTERNAL_H

#define DIRECTORY_INTERNAL_H

struct SquashDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};

STATIC_ASSERT(
		sizeof(struct SquashDirectoryEntry) == SQUASH_SIZEOF_DIRECTORY_ENTRY)

struct SquashDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	// struct SquashDirectoryEntry entries[0]; // [count + 1]
};

STATIC_ASSERT(sizeof(struct SquashDirectoryFragment) ==
		SQUASH_SIZEOF_DIRECTORY_FRAGMENT)

#endif /* end of include guard DIRECTORY_INTERNAL_H */
