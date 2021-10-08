/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : file_context
 * @created     : Thursday Oct 07, 2021 09:23:05 CEST
 */

#include "datablock_context.h"
#include "fragment_context.h"
#include <stdint.h>

#ifndef FILE_CONTEXT_H

#define FILE_CONTEXT_H

struct SquashFileContext {
	struct SquashDatablockContext datablock;
	struct SquashFragmentContext fragment;
	const struct SquashSuperblock *superblock;
	uint32_t fragment_pos;
};

SQUASH_NO_UNUSED int squash_file_init(struct SquashFileContext *context,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode);

SQUASH_NO_UNUSED int squash_file_seek(
		struct SquashFileContext *context, uint64_t seek_pos);

int squash_file_read(struct SquashFileContext *context, uint64_t size);

const uint8_t *squash_file_data(struct SquashFileContext *context);

uint64_t squash_file_size(struct SquashFileContext *context);

int squash_file_cleanup(struct SquashFileContext *context);

#endif /* end of include guard FILE_CONTEXT_H */
