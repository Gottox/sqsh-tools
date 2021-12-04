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
 * @file        : mapper
 * @created     : Sunday Nov 21, 2021 12:30:29 CET
 */

#include "memory_mapper.h"
#include "../error.h"
#include <stdint.h>
#include <string.h>

extern struct HsqsMemoryMapperImpl hsqs_mapper_impl_in_memory;
extern struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap_complete;
extern struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap;

int
hsqs_mapper_init_mmap(struct HsqsMemoryMapper *mapper, const char *path) {
	mapper->impl = &hsqs_mapper_impl_mmap_complete;
	return mapper->impl->init(mapper, path, strlen(path));
}

int
hsqs_mapper_init_static(
		struct HsqsMemoryMapper *mapper, const uint8_t *input, size_t size) {
	mapper->impl = &hsqs_mapper_impl_in_memory;
	return mapper->impl->init(mapper, input, size);
}

int
hsqs_mapper_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size) {
	size_t end_offset;
	size_t archive_size = hsqs_mapper_size(mapper);
	if (offset > archive_size) {
		return -HSQS_ERROR_TODO;
	}
	if (ADD_OVERFLOW(offset, size, &end_offset)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	if (end_offset > archive_size) {
		return -HSQS_ERROR_TODO;
	}
	map->mapper = mapper;
	return mapper->impl->map(map, mapper, offset, size);
}

int
hsqs_mapper_size(const struct HsqsMemoryMapper *mapper) {
	return mapper->impl->size(mapper);
}

int
hsqs_mapper_cleanup(struct HsqsMemoryMapper *mapper) {
	int rv = 0;
	rv = mapper->impl->cleanup(mapper);
	mapper->impl = NULL;
	return rv;
}

int
hsqs_map_resize(struct HsqsMemoryMap *map, size_t new_size) {
	return map->mapper->impl->map_resize(map, new_size);
}

size_t
hsqs_map_size(struct HsqsMemoryMap *map) {
	return map->mapper->impl->map_size(map);
}

const uint8_t *
hsqs_map_data(struct HsqsMemoryMap *map) {
	return map->mapper->impl->map_data(map);
}

int
hsqs_map_unmap(struct HsqsMemoryMap *map) {
	int rv = 0;
	if (map->mapper) {
		rv = map->mapper->impl->unmap(map);
	}
	map->mapper = NULL;
	return rv;
}
