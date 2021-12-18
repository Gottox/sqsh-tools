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
 * @file        : content_context
 * @created     : Thursday Oct 07, 2021 09:23:05 CEST
 */

#include "../compression/buffer.h"
#include <stdint.h>

#ifndef FILE_CONTEXT_H

#define FILE_CONTEXT_H

struct HsqsInodeContext;
struct Hsqs;

struct HsqsFileContext {
	struct Hsqs *hsqs;
	struct HsqsFragmentTableContext *fragment_table;
	struct HsqsInodeContext *inode;
	struct HsqsBuffer buffer;
	struct HsqsMapper *mapper;
	uint64_t seek_pos;
	uint32_t block_size;
};

HSQS_NO_UNUSED int hsqs_content_init(
		struct HsqsFileContext *context, struct HsqsInodeContext *inode);

HSQS_NO_UNUSED int
hsqs_content_seek(struct HsqsFileContext *context, uint64_t seek_pos);

int hsqs_content_read(struct HsqsFileContext *context, uint64_t size);

const uint8_t *hsqs_content_data(struct HsqsFileContext *context);

uint64_t hsqs_content_size(struct HsqsFileContext *context);

int hsqs_content_cleanup(struct HsqsFileContext *context);

#endif /* end of include guard FILE_CONTEXT_H */
