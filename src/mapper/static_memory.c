/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : static_mem
 * @created     : Sunday Nov 21, 2021 16:01:03 CET
 */

#include "memory_mapper.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int
hsqs_mapper_static_mem_init(
		struct HsqsMemoryMapper *mapper, const void *input, size_t size) {
	mapper->data.sm.data = input;
	mapper->data.sm.size = size;
	return 0;
}
static int
hsqs_mapper_static_mem_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size) {
	map->data.sm.data = &mapper->data.sm.data[offset];
	map->data.sm.size = size;
	return 0;
}
static size_t
hsqs_mapper_static_mem_size(struct HsqsMemoryMapper *mapper) {
	return mapper->data.sm.size;
}
static int
hsqs_mapper_static_mem_cleanup(struct HsqsMemoryMapper *mapper) {
	return 0;
}
static int
hsqs_map_static_mem_unmap(struct HsqsMemoryMap *map) {
	map->data.sm.data = NULL;
	map->data.sm.size = 0;
	return 0;
}
static const uint8_t *
hsqs_map_static_mem_data(struct HsqsMemoryMap *map) {
	return map->data.sm.data;
}

static int
hsqs_map_static_mem_resize(struct HsqsMemoryMap *map, size_t new_size) {
	return map->data.sm.size;
}

static size_t
hsqs_map_static_mem_size(struct HsqsMemoryMap *map) {
	return map->data.sm.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_in_memory = {
		.init = hsqs_mapper_static_mem_init,
		.map = hsqs_mapper_static_mem_map,
		.size = hsqs_mapper_static_mem_size,
		.cleanup = hsqs_mapper_static_mem_cleanup,
		.map_data = hsqs_map_static_mem_data,
		.map_resize = hsqs_map_static_mem_resize,
		.map_size = hsqs_map_static_mem_size,
		.unmap = hsqs_map_static_mem_unmap,
};
