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
hsqs_mapper_size(struct HsqsMemoryMapper *mapper) {
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
	rv = map->mapper->impl->unmap(map);
	map->mapper = NULL;
	return rv;
}
