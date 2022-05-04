/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file        : cow
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include "../cache/ref_count.h"
#include "buffer.h"

#ifndef HSQS_COW_H

#define HSQS_COW_H

enum HsqsCowState {
	HSQS_COW_EMPTY,
	HSQS_COW_PASS_THROUGH,
	HSQS_COW_BUFFERED,
};

struct HsqsCowMapping {
	struct HsqsRefCount *rc;
	struct HsqsMapping *mapping;
	size_t offset;
	size_t size;
};

struct HsqsCow {
	enum HsqsCowState state;
	int block_size;
	int compression_id;
	union {
		struct HsqsBuffer buffer;
		struct HsqsCowMapping mapping;
	} content;
};

HSQS_NO_UNUSED int
hsqs_cow_init(struct HsqsCow *cow, int compression_id, int block_size);

HSQS_NO_UNUSED int hsqs_cow_append_block(
		struct HsqsCow *cow, struct HsqsRefCount *mapping,
		const size_t mapping_index, const size_t mapping_size,
		bool is_compressed);

const uint8_t *hsqs_cow_data(const struct HsqsCow *cow);
size_t hsqs_cow_size(const struct HsqsCow *cow);

int hsqs_cow_cleanup(struct HsqsCow *cow);

#endif /* end of include guard HSQS_COW_H */
