/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Monday Sep 06, 2021 14:46:06 CEST
 */

#include "../utils.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef SQUASH_FORMAT_DIRECTORY_H

#define SQUASH_FORMAT_DIRECTORY_H

#define SQUASH_DIRECTORY_ENTRY_SIZE 8
#define SQUASH_DIRECTORY_FRAGMENT_SIZE 12

struct SquashDirectoryEntry;

struct SquashDirectoryFragment;

uint16_t squash_format_directory_entry_offset(
		const struct SquashDirectoryEntry *entry);
int16_t squash_format_directory_entry_inode_offset(
		const struct SquashDirectoryEntry *entry);
uint16_t squash_format_directory_entry_type(
		const struct SquashDirectoryEntry *entry);
uint16_t squash_format_directory_entry_name_size(
		const struct SquashDirectoryEntry *entry);
const uint8_t *squash_format_directory_entry_name(
		const struct SquashDirectoryEntry *entry);

uint32_t squash_format_directory_fragment_count(
		const struct SquashDirectoryFragment *fragment);
uint32_t squash_format_directory_fragment_start(
		const struct SquashDirectoryFragment *fragment);
uint32_t squash_format_directory_fragment_inode_number(
		const struct SquashDirectoryFragment *fragment);
const struct SquashDirectoryEntry *squash_format_directory_fragment_entries(
		const struct SquashDirectoryFragment *fragment);

#endif /* end of include guard SQUASH_FORMAT_DIRECTORY_H */
