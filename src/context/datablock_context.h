/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

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
	const struct SquashSuperblockContext *superblock;
	const struct SquashInodeContext *inode;
	const uint8_t *blocks;
	uint32_t blocks_count;

	const uint8_t *fragment_start;

	uint32_t datablock_index;
	uint32_t datablock_offset;
};

SQUASH_NO_UNUSED int squash_datablock_init(
		struct SquashDatablockContext *file_content,
		const struct SquashInodeContext *inode);

void *squash_datablock_data(const struct SquashDatablockContext *context);

size_t squash_datablock_size(const struct SquashDatablockContext *context);

SQUASH_NO_UNUSED int squash_datablock_seek(
		struct SquashDatablockContext *context, uint64_t seek_pos);

SQUASH_NO_UNUSED int
squash_datablock_read(struct SquashDatablockContext *context, uint64_t size);

int squash_datablock_clean(struct SquashDatablockContext *file_content);

#endif /* end of include guard DATABLOCK_CONTEXT_H */
