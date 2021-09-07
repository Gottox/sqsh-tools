/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Monday Sep 06, 2021 15:10:32 CEST
 */

#include "directory.h"
#include <endian.h>
#include <stdint.h>

struct SquashDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};
CASSERT(sizeof(struct SquashDirectoryEntry) == SQUASH_DIRECTORY_ENTRY_SIZE);

struct SquashDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	// struct SquashDirectoryEntry entries[0]; // [count + 1]
};
CASSERT(sizeof(struct SquashDirectoryFragment) ==
		SQUASH_DIRECTORY_FRAGMENT_SIZE);

uint16_t
squash_format_directory_entry_offset(const struct SquashDirectoryEntry *entry) {
	return le16toh(entry->offset);
}

int16_t
squash_format_directory_entry_inode_offset(
		const struct SquashDirectoryEntry *entry) {
	return le16toh(entry->inode_offset);
}

uint16_t
squash_format_directory_entry_type(const struct SquashDirectoryEntry *entry) {
	return le16toh(entry->type);
}

uint16_t
squash_format_directory_entry_name_size(
		const struct SquashDirectoryEntry *entry) {
	return le16toh(entry->name_size);
}

const uint8_t *
squash_format_directory_entry_name(const struct SquashDirectoryEntry *entry) {
	const uint8_t *tmp = (const uint8_t *)entry;
	return (const uint8_t *)&tmp[sizeof(struct SquashDirectoryEntry)];
}

uint32_t
squash_format_directory_fragment_count(
		const struct SquashDirectoryFragment *fragment) {
	return le32toh(fragment->count);
}
uint32_t
squash_format_directory_fragment_start(
		const struct SquashDirectoryFragment *fragment) {
	return le32toh(fragment->start);
}
uint32_t
squash_format_directory_fragment_inode_number(
		const struct SquashDirectoryFragment *fragment) {
	return le32toh(fragment->inode_number);
}
const struct SquashDirectoryEntry *
squash_format_directory_fragment_entries(
		const struct SquashDirectoryFragment *fragment) {
	const uint8_t *tmp = (const uint8_t *)fragment;
	return (const struct SquashDirectoryEntry *)&tmp[sizeof(struct SquashDirectoryFragment)];
}
