/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mapper
 * @created     : Sunday Nov 21, 2021 12:17:35 CET
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mmap_complete.h"
#include "static_memory.h"

#ifndef MAPPER_H

#define MAPPER_H

struct HsqsMapper;

enum HsqsMapperType {
	HSQS_MAPPER_IN_MEMORY,
	HSQS_MAPPER_MMAP_COMPLETE,
};

struct HsqsMapperMap {
	struct HsqsMapper *mapper;
	const uint8_t *data;
	size_t size;
};

struct HsqsMapperImpl {
	int (*init)(struct HsqsMapper *mapper, const void *input, size_t size);
	int (*map)(
			struct HsqsMapperMap *map, struct HsqsMapper *mapper, off_t offset,
			size_t size);
	int (*unmap)(struct HsqsMapperMap *map);
	int (*cleanup)(struct HsqsMapper *mapper);
};

struct HsqsMapper {
	struct HsqsMapperImpl *impl;
	union {
		struct HsqsMapperMmapComplete mc;
		struct HsqsMapperStaticMemory sm;
	} data;
	size_t size;
};

int hsqs_mapper_init_mmap(struct HsqsMapper *mapper, const char *path);
int hsqs_mapper_init_static(
		struct HsqsMapper *mapper, const uint8_t *input, size_t size);
int hsqs_mapper_map(
		struct HsqsMapperMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size);
int hsqs_mapper_unmap(struct HsqsMapperMap *map);
int hsqs_mapper_size(struct HsqsMapper *mapper);
int hsqs_mapper_cleanup(struct HsqsMapper *mapper);

#endif /* end of include guard MAPPER_H */
