/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : datablock_context
 * @created     : Tuesday Sep 14, 2021 22:39:47 CEST
 */

#include <stdint.h>

#include "../compression/buffer.h"

#ifndef DATABLOCK_CONTEXT_H

#define DATABLOCK_CONTEXT_H

struct SquashDatablockContext {
	struct SquashBuffer buffer;
	const struct SquashSuperblock *superblock;
	const struct SquashInodeContext *inode;
	const uint8_t *blocks;
	uint32_t blocks_count;

	const uint8_t *fragment_start;

	uint32_t datablock_index;
	uint32_t datablock_offset;
};

SQUASH_NO_UNUSED int squash_datablock_init(
		struct SquashDatablockContext *file_content,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode);

void *squash_datablock_data(const struct SquashDatablockContext *context);

size_t squash_datablock_size(const struct SquashDatablockContext *context);

SQUASH_NO_UNUSED int squash_datablock_seek(
		struct SquashDatablockContext *context, uint64_t seek_pos);

SQUASH_NO_UNUSED int squash_datablock_read(
		struct SquashDatablockContext *context, uint64_t size);

int squash_datablock_clean(struct SquashDatablockContext *file_content);

#endif /* end of include guard DATABLOCK_CONTEXT_H */
