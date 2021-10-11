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
	const struct SquashSuperblockContext *superblock;
	uint32_t fragment_pos;
};

SQUASH_NO_UNUSED int squash_file_init(struct SquashFileContext *context,
		const struct SquashInodeContext *inode);

SQUASH_NO_UNUSED int squash_file_seek(
		struct SquashFileContext *context, uint64_t seek_pos);

int squash_file_read(struct SquashFileContext *context, uint64_t size);

const uint8_t *squash_file_data(struct SquashFileContext *context);

uint64_t squash_file_size(struct SquashFileContext *context);

int squash_file_cleanup(struct SquashFileContext *context);

#endif /* end of include guard FILE_CONTEXT_H */
