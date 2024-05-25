/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         map_slice.c
 */

#include <sqsh_mapper_private.h>

#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>

int
sqsh__map_slice_init(
		struct SqshMapSlice *mapping, struct SqshMapper *mapper,
		sqsh_index_t offset, size_t size) {
	size_t end_offset;
	size_t archive_size = sqsh_mapper_size(mapper);
	if (offset > archive_size) {
		return -SQSH_ERROR_SIZE_MISMATCH;
	}
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (end_offset > archive_size) {
		return -SQSH_ERROR_SIZE_MISMATCH;
	}
	mapping->mapper = mapper;
	mapping->size = size;
	return mapper->impl->map(mapper, offset, size, &mapping->data);
}

const uint8_t *
sqsh__map_slice_data(const struct SqshMapSlice *mapping) {
	return mapping->data;
}

size_t
sqsh__map_slice_size(const struct SqshMapSlice *mapping) {
	return mapping->size;
}

int
sqsh__map_slice_cleanup(struct SqshMapSlice *mapping) {
	int rv = 0;

	if (mapping->mapper) {
		uint8_t *data = mapping->data;
		size_t size = sqsh__map_slice_size(mapping);
		rv = mapping->mapper->impl->unmap(mapping->mapper, data, size);
	}
	mapping->mapper = NULL;
	return rv;
}
