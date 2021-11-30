/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : static_mem
 * @created     : Sunday Nov 21, 2021 16:01:03 CET
 */

#include "mapper.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

static int
hsqs_mapper_static_mem_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	mapper->data.sm.data = input;
	mapper->size = size;
	return 0;
}
static int
hsqs_mapper_static_mem_map(
		struct HsqsMapperMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size) {
	map->data = &mapper->data.sm.data[offset];
	map->size = size;
	return 0;
}
static int
hsqs_mapper_static_mem_unmap(struct HsqsMapperMap *map) {
	map->data = NULL;
	map->size = 0;
	return 0;
}
static int
hsqs_mapper_static_mem_cleanup(struct HsqsMapper *mapper) {
	return 0;
}

struct HsqsMapperImpl hsqs_mapper_impl_in_memory = {
		.init = hsqs_mapper_static_mem_init,
		.map = hsqs_mapper_static_mem_map,
		.unmap = hsqs_mapper_static_mem_unmap,
		.cleanup = hsqs_mapper_static_mem_cleanup,
};
