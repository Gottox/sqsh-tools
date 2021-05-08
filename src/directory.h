/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include <stdint.h>

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

#endif /* end of include guard DIRECTORY_H */
