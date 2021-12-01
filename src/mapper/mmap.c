/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mmap
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
hsqs_mapper_mmap_init(
		struct HsqsMemoryMapper *mapper, const void *input, size_t size) {
	int rv = 0;
	int fd = -1;
	struct stat st = {0};

	fd = open(input, 0);
	if (fd < 0) {
		rv = -errno;
		goto out;
	}

	if (fstat(fd, &st) < 0) {
		rv = -errno;
		goto out;
	}
	mapper->data.mm.fd = fd;
	mapper->data.mm.size = st.st_size;
	fd = -1;

out:
	if (fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
hsqs_mapper_mmap_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size) {
	uint8_t *file_map = MAP_FAILED;
	file_map = mmap(
			NULL, size, PROT_READ, MAP_PRIVATE, mapper->data.mm.fd, offset);
	if (file_map == MAP_FAILED) {
		return -errno;
	}
	map->data.mm.data = file_map;
	map->data.mm.size = size;
	return 0;
}
static int
hsqs_mapper_mmap_cleanup(struct HsqsMemoryMapper *mapper) {
	close(mapper->data.mm.fd);
	return 0;
}
static size_t
hsqs_mapper_mmap_size(struct HsqsMemoryMapper *mapper) {
	return mapper->data.mm.size;
}

static int
hsqs_map_mmap_unmap(struct HsqsMemoryMap *map) {
	int rv;
	rv = munmap(map->data.mm.data, map->data.mm.size);

	map->data.mm.data = NULL;
	map->data.mm.size = 0;
	return rv;
}
static const uint8_t *
hsqs_map_mmap_data(struct HsqsMemoryMap *map) {
	return map->data.mm.data;
}
static size_t
hsqs_map_mmap_size(struct HsqsMemoryMap *map) {
	return map->data.mm.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap = {
		.init = hsqs_mapper_mmap_init,
		.map = hsqs_mapper_mmap_map,
		.size = hsqs_mapper_mmap_size,
		.cleanup = hsqs_mapper_mmap_cleanup,
		.map_data = hsqs_map_mmap_data,
		.map_size = hsqs_map_mmap_size,
		.unmap = hsqs_map_mmap_unmap,
};
