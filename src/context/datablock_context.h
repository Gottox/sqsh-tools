/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : datablock_context
 * @created     : Tuesday Sep 14, 2021 22:39:47 CEST
 */

#include <stdint.h>

#include "inode_context.h"

#ifndef DATABLOCK_CONTEXT_H

#define DATABLOCK_CONTEXT_H

struct SquashDatablockContext {
	struct SquashInodeContext *inode;
	struct SquashBuffer buffer;
	struct SquashSuperblock *superblock;
	const uint8_t *datablock_start;
	const uint8_t *fragment_start;
	uint64_t blocks_start;
	off_t block_sizes_index;
};

int squash_datablock_init(struct SquashDatablockContext *datablock,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode);
int squash_datablock_clean(struct SquashDatablockContext *datablock);

#endif /* end of include guard DATABLOCK_CONTEXT_H */
