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

/**
 * @brief The SqshBuffer struct is a buffer for arbitrary data.
 *
 * The buffer takes care about resizing and freeing the memory managed by The
 * buffer.
 */
struct SqshBuffer {
	uint8_t *data;
	size_t size;
	size_t capacity;
};

/**
 * @brief sqsh_buffer_init initializes a SqshBuffer.
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to initialize.
 */
SQSH_NO_UNUSED int sqsh_buffer_init(struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_add_size tells SqshBuffer to increase the buffer by
 * additional_size
 * @memberof SqshBuffer
 *
 * Please be aware, that the buffer needs to be allocated by
 * sqsh_buffer_add_capacity before. Otherwise the function behavior is
 * undefined.
 *
 * @param buffer The SqshBuffer to increase.
 * @param additional_size The additional size to increase the buffer.
 */
SQSH_NO_UNUSED int
sqsh_buffer_add_size(struct SqshBuffer *buffer, size_t additional_size);

/**
 * @brief sqsh_buffer_add_size allocates additional memory for the SqshBuffer
 * and sets additional_buffer to the beginning of the additional memory.
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to free.
 * @param additional_buffer The pointer to the additional memory.
 * @param additional_size The size of the additional memory.
 */
SQSH_NO_UNUSED int sqsh_buffer_add_capacity(
		struct SqshBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size);

/**
 * @brief sqsh_buffer_append
 * @memberof SqshBuffer
 *
 * @param buffer The SqshBuffer to append to.
 * @param source The data to append.
 * @param source_size The size of the data to append.
 */
SQSH_NO_UNUSED int sqsh_buffer_append(
		struct SqshBuffer *buffer, const uint8_t *source,
		const size_t source_size);

/**
 * @brief sqsh_buffer_drain resets the buffer size to 0.
 * @memberof SqshBuffer
 *
 * This does not free the memory allocated by the buffer so that
 * the buffer can be reused.
 *
 * @param buffer The SqshBuffer to drain.
 */

void sqsh_buffer_drain(struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_data returns the data of the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to get the data from.
 * @return a pointer to the data of the SqshBuffer.
 */
const uint8_t *sqsh_buffer_data(const struct SqshBuffer *buffer);
/**
 * @brief sqsh_buffer_size returns the size of the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to get the size from.
 * @return the size of the SqshBuffer.
 */
size_t sqsh_buffer_size(const struct SqshBuffer *buffer);

/**
 * @brief sqsh_buffer_cleanup frees the memory managed by the SqshBuffer.
 * @memberof SqshBuffer
 * @param buffer The SqshBuffer to cleanup.
 */
int sqsh_buffer_cleanup(struct SqshBuffer *buffer);

#endif /* end of include guard SQSH_BUFFER_H */
