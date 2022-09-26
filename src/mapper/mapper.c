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
 * @file         mapper.c
 */

#include "mapper.h"
#include "../error.h"
#include <stdint.h>
#include <string.h>

int
sqsh_mapper_init(
		struct SqshMapper *mapper, struct SqshMemoryMapperImpl *impl,
		const void *input, size_t size) {
	mapper->impl = impl;
	return mapper->impl->init(mapper, input, size);
}

int
sqsh_mapper_init_mmap(struct SqshMapper *mapper, const char *path) {
	return sqsh_mapper_init(mapper, &sqsh_mapper_impl_mmap, path, strlen(path));
}

int
sqsh_mapper_init_static(
		struct SqshMapper *mapper, const uint8_t *input, size_t size) {
	return sqsh_mapper_init(mapper, &sqsh_mapper_impl_static, input, size);
}

int
sqsh_mapper_map(
		struct SqshMapping *mapping, struct SqshMapper *mapper,
		sqsh_index_t offset, size_t size) {
	size_t end_offset;
	size_t archive_size = sqsh_mapper_size(mapper);
	if (offset > archive_size) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	if (ADD_OVERFLOW(offset, size, &end_offset)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	if (end_offset > archive_size) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	mapping->mapper = mapper;
	return mapper->impl->mapping(mapping, offset, size);
}

size_t
sqsh_mapper_size(const struct SqshMapper *mapper) {
	return mapper->impl->size(mapper);
}

int
sqsh_mapper_cleanup(struct SqshMapper *mapper) {
	int rv = 0;
	if (mapper->impl != NULL) {
		rv = mapper->impl->cleanup(mapper);
		mapper->impl = NULL;
	}
	return rv;
}

int
sqsh_mapping_resize(struct SqshMapping *mapping, size_t new_size) {
	if (new_size <= sqsh_mapping_size(mapping)) {
		return 0;
	} else {
		return mapping->mapper->impl->map_resize(mapping, new_size);
	}
}

size_t
sqsh_mapping_size(struct SqshMapping *mapping) {
	return mapping->mapper->impl->map_size(mapping);
}

const uint8_t *
sqsh_mapping_data(const struct SqshMapping *mapping) {
	return mapping->mapper->impl->map_data(mapping);
}

int
sqsh_mapping_unmap(struct SqshMapping *mapping) {
	int rv = 0;
	if (mapping->mapper) {
		rv = mapping->mapper->impl->unmap(mapping);
	}
	mapping->mapper = NULL;
	return rv;
}
