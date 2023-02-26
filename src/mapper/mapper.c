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
 * @file         mapper.c
 */

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

int
sqsh__mapper_init(
		struct SqshMapper *mapper, const void *input,
		const struct SqshConfig *config) {
	int rv = 0;
	size_t size = config->source_size;
	if (config->source_mapper) {
		mapper->impl = config->source_mapper;
	} else {
		mapper->impl = sqsh_mapper_impl_mmap;
	}

	if (config->mapper_block_size) {
		mapper->block_size = config->mapper_block_size;
	} else {
		mapper->block_size = mapper->impl->block_size_hint;
	}

	rv = mapper->impl->init(mapper, input, &size);
	if (rv < 0) {
		return rv;
	}

	mapper->archive_size = size;

	return rv;
}

int
sqsh__mapping_init(
		struct SqshMapping *mapping, struct SqshMapper *mapper,
		sqsh_index_t offset, size_t size) {
	size_t end_offset;
	size_t archive_size = sqsh__mapper_size(mapper);
	if (offset > archive_size) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (end_offset > archive_size) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}
	mapping->mapper = mapper;
	mapping->offset = offset;
	mapping->size = size;
	return mapper->impl->map(mapping);
}

size_t
sqsh__mapper_block_size(const struct SqshMapper *mapper) {
	return mapper->block_size;
}

size_t
sqsh__mapper_size(const struct SqshMapper *mapper) {
	return mapper->archive_size;
}

int
sqsh__mapper_cleanup(struct SqshMapper *mapper) {
	int rv = 0;
	if (mapper->impl != NULL) {
		rv = mapper->impl->cleanup(mapper);
		mapper->impl = NULL;
	}
	return rv;
}

const uint8_t *
sqsh__mapping_data(const struct SqshMapping *mapping) {
	return mapping->mapper->impl->map_data(mapping);
}

int
sqsh__mapping_cleanup(struct SqshMapping *mapping) {
	int rv = 0;
	if (mapping->mapper) {
		rv = mapping->mapper->impl->unmap(mapping);
	}
	mapping->mapper = NULL;
	return rv;
}
