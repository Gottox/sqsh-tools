/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mapper
 * @created     : Sunday Nov 21, 2021 12:17:35 CET
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mmap.h"
#include "mmap_complete.h"
#include "static_memory.h"

#ifndef MEMORY_MAPPER_H

#define MEMORY_MAPPER_H

struct HsqsMemoryMapper;

struct HsqsMemoryMap {
	struct HsqsMemoryMapper *mapper;
	union {
		struct HsqsMapMmapComplete mc;
		struct HsqsMapMmap mm;
		struct HsqsMapStaticMemory sm;
	} data;
};

struct HsqsMemoryMapperImpl {
	int (*init)(
			struct HsqsMemoryMapper *mapper, const void *input, size_t size);
	int (*map)(
			struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
			off_t offset, size_t size);
	size_t (*size)(struct HsqsMemoryMapper *mapper);
	int (*cleanup)(struct HsqsMemoryMapper *mapper);
	const uint8_t *(*map_data)(struct HsqsMemoryMap *map);
	int (*map_resize)(struct HsqsMemoryMap *map, size_t new_size);
	size_t (*map_size)(struct HsqsMemoryMap *map);
	int (*unmap)(struct HsqsMemoryMap *map);
};

struct HsqsMemoryMapper {
	struct HsqsMemoryMapperImpl *impl;
	union {
		struct HsqsMapperMmapComplete mc;
		struct HsqsMapperMmap mm;
		struct HsqsMapperStaticMemory sm;
	} data;
};

int hsqs_mapper_init_mmap(struct HsqsMemoryMapper *mapper, const char *path);
int hsqs_mapper_init_static(
		struct HsqsMemoryMapper *mapper, const uint8_t *input, size_t size);
int hsqs_mapper_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size);
int hsqs_mapper_size(struct HsqsMemoryMapper *mapper);
int hsqs_mapper_cleanup(struct HsqsMemoryMapper *mapper);
size_t hsqs_map_size(struct HsqsMemoryMap *map);
int hsqs_map_resize(struct HsqsMemoryMap *map, size_t new_size);
const uint8_t *hsqs_map_data(struct HsqsMemoryMap *map);
int hsqs_map_unmap(struct HsqsMemoryMap *map);

#endif /* end of include guard MEMORY_MAPPER_H */
