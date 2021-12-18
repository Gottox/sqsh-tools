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
 * @file        : fragment_table_context
 * @created     : Wednesday Dec 01, 2021 17:22:03 CET
 */

#include "../utils.h"
#include "table_context.h"

#ifndef FRAGMENT_TABLE_CONTEXT_H

#define FRAGMENT_TABLE_CONTEXT_H

struct HsqsSuperblockContext;
struct HsqsInodeContext;
struct HsqsBuffer;

struct HsqsFragmentTableContext {
	const struct HsqsSuperblockContext *superblock;
	struct HsqsTableContext table;
	struct HsqsMapper *mapper;
};

HSQS_NO_UNUSED int hsqs_fragment_table_init(
		struct HsqsFragmentTableContext *context, struct Hsqs *hsqs);

HSQS_NO_UNUSED int hsqs_fragment_table_to_buffer(
		struct HsqsFragmentTableContext *context,
		const struct HsqsInodeContext *inode, struct HsqsBuffer *buffer);

int hsqs_fragment_table_cleanup(struct HsqsFragmentTableContext *context);

#endif /* end of include guard FRAGMENT_TABLE_CONTEXT_H */
