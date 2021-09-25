/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include "../utils.h"
#include "metablock_context.h"

#ifndef SQUASH_DIRECTORY_CONTEXT_H

#define SQUASH_DIRECTORY_CONTEXT_H

struct SquashInodeContext;
struct SquashSuperblock;

struct SquashDirectoryContext {
	const struct SquashSuperblock *superblock;
	struct SquashInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;
};

struct SquashDirectoryIterator {
	const struct SquashDirectoryFragment *fragments;
	struct SquashDirectoryContext *directory;
	struct SquashMetablockContext extract;
	size_t remaining_entries;
	off_t current_fragment_offset;
	off_t next_offset;
};

SQUASH_NO_UNUSED int squash_directory_init(
		struct SquashDirectoryContext *directory,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode);
SQUASH_NO_UNUSED int squash_directory_iterator_init(
		struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory);
SQUASH_NO_UNUSED const struct SquashDirectoryFragment *
squash_directory_iterator_current_fragment(
		const struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED const struct SquashDirectoryEntry *
squash_directory_iterator_next(struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED const struct SquashDirectoryEntry *
squash_directory_iterator_lookup(struct SquashDirectoryIterator *iterator,
		const char *name, const size_t name_len);
int squash_directory_iterator_cleanup(struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_entry_name_size(
		const struct SquashDirectoryEntry *entry);
const char *squash_directory_entry_name(
		const struct SquashDirectoryEntry *entry);
SQUASH_NO_UNUSED int squash_directory_entry_name_dup(
		const struct SquashDirectoryEntry *entry, char **name_buffer);

int squash_directory_cleanup(struct SquashDirectoryContext *directory);
#endif /* end of include guard SQUASH_DIRECTORY_CONTEXT_H */
