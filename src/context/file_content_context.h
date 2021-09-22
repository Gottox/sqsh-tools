/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : file_content_context
 * @created     : Tuesday Sep 14, 2021 22:39:47 CEST
 */

#include <stdint.h>

#include "inode_context.h"

#ifndef DATABLOCK_CONTEXT_H

#define DATABLOCK_CONTEXT_H

struct SquashFileContentContext {
	struct SquashInodeContext *inode;
	struct SquashBuffer buffer;
	const struct SquashSuperblock *superblock;
	const uint8_t *datablock_start;
	const uint8_t *fragment_start;

	uint32_t datablock_index;
	uint32_t datablock_offset;
};

SQUASH_NO_UNUSED int squash_file_content_init(
		struct SquashFileContentContext *file_content,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode);

void *squash_file_content_data(const struct SquashFileContentContext *context);

size_t squash_file_content_size(const struct SquashFileContentContext *context);

SQUASH_NO_UNUSED int squash_file_content_read(
		struct SquashFileContentContext *context, uint64_t size);

int squash_file_content_clean(struct SquashFileContentContext *file_content);

#endif /* end of include guard DATABLOCK_CONTEXT_H */
