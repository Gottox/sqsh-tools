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
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:28:28 CEST
 */

#include "table_context.h"
#include <stdint.h>

#ifndef FRAGMENT_CONTEXT_H

#define FRAGMENT_CONTEXT_H

struct HsqsInodeContext;

struct HsqsFragmentContext {
	const struct HsqsSuperblockContext *superblock;
	const struct HsqsInodeContext *inode;
	// TODO: This table should be part of struct Hsqs.
	struct HsqsTableContext table;
	const struct HsqsFragment *fragment;
	struct HsqsBuffer buffer;
};

HSQS_NO_UNUSED int hsqs_fragment_init(
		struct HsqsFragmentContext *fragment,
		const struct HsqsInodeContext *inode);

uint64_t hsqs_fragment_start(struct HsqsFragmentContext *fragment);

uint32_t hsqs_fragment_size(struct HsqsFragmentContext *fragment);

HSQS_NO_UNUSED int hsqs_fragment_read(struct HsqsFragmentContext *fragment);

const uint8_t *hsqs_fragment_data(struct HsqsFragmentContext *fragment);

int hsqs_fragment_clean(struct HsqsFragmentContext *fragment);

#endif /* end of include guard FRAGMENT_CONTEXT_H */
