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
 * @file         sqsh_compression.h
 */

#ifndef SQSH_COMPRESSION_H

#define SQSH_COMPRESSION_H

#include <stddef.h>
#include <stdint.h>

union SqshCompressionOptions;
struct SqshSuperblockContext;
struct SqshBuffer;

struct SqshCompressionImplementation {
	int (*extract)(
			const union SqshCompressionOptions *options, size_t options_size,
			uint8_t *target, size_t *target_size, const uint8_t *compressed,
			const size_t compressed_size);
};

struct SqshCompression {
	const struct SqshCompressionImplementation *impl;
	size_t block_size;
};

int sqsh_compression_init(
		struct SqshCompression *compression, int compression_id,
		size_t block_size);

int sqsh_compression_decompress_to_buffer(
		const struct SqshCompression *compression, struct SqshBuffer *buffer,
		const uint8_t *compressed, const size_t compressed_size);

int sqsh_compression_cleanup(struct SqshCompression *compression);

#endif /* end of include guard SQSH_COMPRESSION_H */
