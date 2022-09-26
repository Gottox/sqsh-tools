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
 * @file         buffer.c
 */

#include "buffer.h"
#include "../context/superblock_context.h"
#include "../data/metablock.h"
#include "../data/superblock_internal.h"
#include "../error.h"
#include "../utils.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int
hsqs_buffer_init(
		struct HsqsBuffer *buffer) {
	int rv = 0;

	buffer->data = NULL;
	buffer->size = 0;

	return rv;
}

int
hsqs_buffer_add_capacity(
		struct HsqsBuffer *buffer, uint8_t **additional_buffer,
		size_t additional_size) {
	const size_t buffer_size = buffer->size;
	size_t new_capacity;

	if (ADD_OVERFLOW(buffer_size, additional_size, &new_capacity)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	buffer->data = realloc(buffer->data, new_capacity);
	if (buffer->data == NULL) {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
	if (additional_buffer != NULL) {
		*additional_buffer = &buffer->data[buffer_size];
	}
	return new_capacity;
}

int
hsqs_buffer_add_size(struct HsqsBuffer *buffer, size_t additional_size) {
	const size_t buffer_size = buffer->size;
	size_t new_size;
	if (ADD_OVERFLOW(buffer_size, additional_size, &new_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	buffer->size = new_size;
	return 0;
}

int
hsqs_buffer_append(
		struct HsqsBuffer *buffer, const uint8_t *source,
		const size_t size) {
	int rv = 0;
	uint8_t *additional_buffer;

	rv = hsqs_buffer_add_capacity(buffer, &additional_buffer, size);
	if (rv < 0) {
		return rv;
	}

	memcpy(additional_buffer, source, size);
	rv = hsqs_buffer_add_size(buffer, size);
	return rv;
}

const uint8_t *
hsqs_buffer_data(const struct HsqsBuffer *buffer) {
	return buffer->data;
}
size_t
hsqs_buffer_size(const struct HsqsBuffer *buffer) {
	return buffer->size;
}

int
hsqs_buffer_cleanup(struct HsqsBuffer *buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->size = 0;
	return 0;
}
