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
 * @author       Enno Boland (mail@eboland.de)
 * @file         metablock_context.h
 */

#include "../cache/lru_hashmap.h"
#include "../mapper/mapper.h"
#include "../utils.h"
#include <stdint.h>

#ifndef METABLOCK_CONTEXT_H

#define METABLOCK_CONTEXT_H

#define HSQS_METABLOCK_BLOCK_SIZE 8192

struct Hsqs;

struct HsqsSuperblockContext;
struct HsqsBuffer;

struct HsqsMetablockContext {
	struct Hsqs *hsqs;
	uint64_t address;
	struct HsqsRefCount *buffer_ref;
	struct HsqsBuffer *buffer;
	struct HsqsMapping mapping;
};

int hsqs_metablock_init(
		struct HsqsMetablockContext *context, struct Hsqs *hsqs,
		uint64_t address);

uint32_t
hsqs_metablock_compressed_size(const struct HsqsMetablockContext *context);

int hsqs_metablock_read(struct HsqsMetablockContext *context);

HSQS_NO_UNUSED int hsqs_metablock_to_buffer(
		struct HsqsMetablockContext *context, struct HsqsBuffer *buffer);

int hsqs_metablock_cleanup(struct HsqsMetablockContext *context);

#endif /* end of include guard METABLOCK_CONTEXT_H */
