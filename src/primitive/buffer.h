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
 * @file         buffer.h
 */

#include "../utils.h"

#include <stdbool.h>

#ifndef SQSH_BUFFER_H

#define SQSH_BUFFER_H

struct SqshSuperblockContext;

struct SqshBuffer {
	uint8_t *data;
	size_t size;
};

SQSH_NO_UNUSED int sqsh_buffer_init(struct SqshBuffer *buffer);

SQSH_NO_UNUSED int
sqsh_buffer_add_size(struct SqshBuffer *buffer, size_t additional_size);
SQSH_NO_UNUSED int sqsh_buffer_add_capacity(
		struct SqshBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size);

SQSH_NO_UNUSED int sqsh_buffer_append(
		struct SqshBuffer *buffer, const uint8_t *source,
		const size_t source_size);

const uint8_t *sqsh_buffer_data(const struct SqshBuffer *buffer);
size_t sqsh_buffer_size(const struct SqshBuffer *buffer);

int sqsh_buffer_cleanup(struct SqshBuffer *buffer);

#endif /* end of include guard SQSH_BUFFER_H */
