/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         buffering_compression.c
 */

#include "../../include/sqsh_compression_private.h"

SQSH_STATIC_ASSERT(
		sizeof(sqsh__compression_context_t) >=
		sizeof(struct SqshBufferingCompression));

int
sqsh__buffering_compression_init(
		void *context, uint8_t *target, size_t target_size) {
	(void)target;
	(void)target_size;
	int rv = 0;
	struct SqshBufferingCompression *buffering = context;

	rv = sqsh__buffer_init(&buffering->buffer);
	if (rv < 0) {
		goto out;
	}
	buffering->compressed = NULL;
	buffering->compressed_size = 0;

out:
	return rv;
}
int
sqsh__buffering_compression_decompress(
		void *context, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	struct SqshBufferingCompression *buffering = context;
	if (buffering->compressed == NULL &&
		sqsh__buffer_size(&buffering->buffer) == 0) {
		buffering->compressed = compressed;
		buffering->compressed_size = compressed_size;
		return 0;
	} else if (sqsh__buffer_size(&buffering->buffer) == 0) {
		rv = sqsh__buffer_append(
				&buffering->buffer, buffering->compressed,
				buffering->compressed_size);
		if (rv < 0) {
			return rv;
		}
	}

	rv = sqsh__buffer_append(&buffering->buffer, compressed, compressed_size);
	if (rv < 0) {
		return rv;
	}
	buffering->compressed = sqsh__buffer_data(&buffering->buffer);
	buffering->compressed_size = sqsh__buffer_size(&buffering->buffer);

	return rv;
}

const uint8_t *
sqsh__buffering_compression_data(void *context) {
	return ((struct SqshBufferingCompression *)context)->compressed;
}

size_t
sqsh__buffering_compression_size(void *context) {
	return ((struct SqshBufferingCompression *)context)->compressed_size;
}

int
sqsh__buffering_compression_cleanup(void *context) {
	struct SqshBufferingCompression *buffering = context;
	return sqsh__buffer_cleanup(&buffering->buffer);
}
