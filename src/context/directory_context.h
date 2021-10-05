/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include "../utils.h"
#include "inode_context.h"
#include <stdint.h>

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
	off_t current_offset;
};

SQUASH_NO_UNUSED int squash_directory_init(
		struct SquashDirectoryContext *directory,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode);
SQUASH_NO_UNUSED int squash_directory_iterator_init(
		struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory);
SQUASH_NO_UNUSED int squash_directory_iterator_next(
		struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_lookup(
		struct SquashDirectoryIterator *iterator, const char *name,
		const size_t name_len);
int squash_directory_iterator_name_size(
		const struct SquashDirectoryIterator *iterator);
uint64_t squash_directory_iterator_inode_ref(
		const struct SquashDirectoryIterator *iterator);
enum SquashInodeContextType squash_directory_iterator_inode_type(
		const struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_inode_load(
		const struct SquashDirectoryIterator *iterator,
		struct SquashInodeContext *inode);
const char *squash_directory_iterator_name(
		const struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_name_dup(
		const struct SquashDirectoryIterator *iterator, char **name_buffer);
int squash_directory_iterator_cleanup(struct SquashDirectoryIterator *iterator);

int squash_directory_cleanup(struct SquashDirectoryContext *directory);
#endif /* end of include guard SQUASH_DIRECTORY_CONTEXT_H */
