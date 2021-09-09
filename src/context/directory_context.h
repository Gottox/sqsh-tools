/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include "../extract.h"
#include "../format/directory.h"

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
	struct SquashDirectoryContext *directory;
	struct SquashDirectoryFragment *fragments;
	struct SquashExtract extract;
	off_t current_fragment_offset;
	size_t remaining_entries;
	off_t next_offset;
};

int squash_directory_init(struct SquashDirectoryContext *directory,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode);
const struct SquashDirectoryEntry *squash_directory_lookup(
		struct SquashDirectoryContext *directory, const char *name);
int squash_directory_iterator_init(struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory);
const struct SquashDirectoryEntry *squash_directory_iterator_next(
		struct SquashDirectoryIterator *iterator);
int squash_directory_iterator_clean(struct SquashDirectoryIterator *iterator);
int squash_directory_entry_name_size(const struct SquashDirectoryEntry *entry);
int squash_directory_entry_name(
		const struct SquashDirectoryEntry *entry, char **name_buffer);

int squash_directory_cleanup(struct SquashDirectoryContext *directory);
#endif /* end of include guard SQUASH_DIRECTORY_CONTEXT_H */
