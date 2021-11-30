/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mapper
 * @created     : Sunday Nov 21, 2021 12:30:29 CET
 */

#include "mapper.h"
#include "../error.h"
#include <string.h>

extern struct HsqsMapperImpl hsqs_mapper_impl_in_memory;
extern struct HsqsMapperImpl hsqs_mapper_impl_mmap_complete;

int
hsqs_mapper_init_mmap(
		struct HsqsMapper *mapper, const char *path) {
	mapper->impl = &hsqs_mapper_impl_mmap_complete;
	return hsqs_mapper_impl_mmap_complete.init(mapper, path, strlen(path));
}

int
hsqs_mapper_init_static(
		struct HsqsMapper *mapper, const uint8_t *input, size_t size) {
	mapper->impl = &hsqs_mapper_impl_in_memory;
	return hsqs_mapper_impl_in_memory.init(mapper, input, size);
}

int
hsqs_mapper_map(
		struct HsqsMapperMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size) {
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
hsqs_mapper_unmap(struct HsqsMapperMap *map) {
	return map->mapper->impl->unmap(map);
}

int hsqs_mapper_size(struct HsqsMapper *mapper) {
	return mapper->size;
}

int
hsqs_mapper_cleanup(struct HsqsMapper *mapper) {
	return mapper->impl->cleanup(mapper);
}
