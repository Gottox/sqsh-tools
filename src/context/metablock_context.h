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

#include "../mapper/mapper.h"
#include "../primitive/buffer.h"
#include "../utils.h"
#include <stdint.h>

#ifndef METABLOCK_CONTEXT_H

#define METABLOCK_CONTEXT_H

#define SQSH_METABLOCK_BLOCK_SIZE 8192

struct Sqsh;

struct SqshSuperblockContext;
struct SqshBuffer;

struct SqshMetablockContext {
	struct SqshMapping mapping;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
};

int sqsh_metablock_init(
		struct SqshMetablockContext *context, struct Sqsh *sqsh,
		uint64_t address);

uint32_t
sqsh_metablock_compressed_size(const struct SqshMetablockContext *context);

SQSH_NO_UNUSED int sqsh_metablock_to_buffer(
		struct SqshMetablockContext *context, struct SqshBuffer *buffer);

int sqsh_metablock_cleanup(struct SqshMetablockContext *context);

#endif /* end of include guard METABLOCK_CONTEXT_H */
