/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include <stdint.h>
#include <stdlib.h>

#include "../extract.h"

#ifndef INODE_H

#define INODE_H

struct Squash;
struct SquashInode;
struct SquashInodeTable;

struct SquashInodeContext {
	struct SquashInode *inode;
	struct SquashExtract extract;
};

int squash_inode_load_ref(struct SquashInodeContext *inode,
		struct Squash *squash, uint64_t inode_ref);

int squash_inode_load(struct SquashInodeContext *inode, struct Squash *squash,
		int block, int offset);

uint32_t squash_inode_hard_link_count(struct SquashInodeContext *inode);

int squash_inode_cleanup(struct SquashInodeContext *inode);

#endif /* end of include guard INODE_H */
