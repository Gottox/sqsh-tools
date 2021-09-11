/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include <stdint.h>
#include <stdlib.h>

#include "../extract.h"

#ifndef SQUASH_INODE_CONTEXT_H

#define SQUASH_INODE_CONTEXT_H

struct SquashSuperblock;
struct SquashInode;
struct SquashInodeTable;

struct SquashInodeContext {
	struct SquashInode *inode;
	struct SquashExtract extract;
};

struct SquashInodeDirectoryIndexIterator {
	struct SquashInodeContext *inode;
	const struct SquashInodeDirectoryIndex *indices;
	size_t remaining_entries;
	off_t offset;
};

int squash_inode_load(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, uint64_t inode_ref);

uint32_t squash_inode_hard_link_count(struct SquashInodeContext *inode);

int squash_inode_cleanup(struct SquashInodeContext *inode);

int squash_inode_directory_iterator_init(
		struct SquashInodeDirectoryIndexIterator *iterator,
		struct SquashInodeContext *inode);
const struct SquashInodeDirectoryIndex *
squash_inode_directory_index_iterator_next(
		struct SquashInodeDirectoryIndexIterator *iterator);
int squash_inode_directory_iterator_clean(
		struct SquashInodeDirectoryIndexIterator *iterator);

void squash_inode_ref_to_block(
		uint64_t ref, uint32_t *block_index, uint16_t *offset);
uint64_t squash_inode_ref_from_block(uint32_t block_index, uint16_t offset);

#endif /* end of include guard SQUASH_INODE_CONTEXT_H */
