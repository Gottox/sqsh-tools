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
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include "../utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef EXTRACTOR_H

#define EXTRACTOR_H

struct SquashSuperblockContext;

struct SquashBuffer {
	const union SquashCompressionOptions *options;
	const struct SquashCompressionImplementation *impl;
	int block_size;
	uint8_t *data;
	size_t size;
};

SQUASH_NO_UNUSED int squash_buffer_new(struct SquashBuffer **context,
		const struct SquashSuperblockContext *superblock, int block_size);

SQUASH_NO_UNUSED int squash_buffer_init(struct SquashBuffer *compression,
		const struct SquashSuperblockContext *superblock, int block_size);

SQUASH_NO_UNUSED int squash_buffer_append(struct SquashBuffer *compression,
		const uint8_t *source, const size_t source_size, bool is_compressed);

const uint8_t *squash_buffer_data(const struct SquashBuffer *buffer);
size_t squash_buffer_size(const struct SquashBuffer *buffer);

int squash_buffer_cleanup(struct SquashBuffer *compression);

int squash_buffer_free(struct SquashBuffer *compression);

#endif /* end of include guard EXTRACTOR_H */
