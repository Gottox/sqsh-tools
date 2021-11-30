/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : mmap_complete
 * @created     : Sunday Nov 21, 2021 16:01:03 CET
 */

#include "mapper.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

static int
hsqs_mapper_mmap_complete_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	int rv = 0;
	int fd = -1;
	uint8_t *file_map = MAP_FAILED;
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

	mapper->data.mc.data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_map == MAP_FAILED) {
		rv = -errno;
	}
	mapper->size = st.st_size;

out:
	if (fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
hsqs_mapper_mmap_complete_map(
		struct HsqsMapperMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size) {
	map->data = &mapper->data.mc.data[offset];
	map->size = size;
	return 0;
}
static int
hsqs_mapper_mmap_complete_unmap(struct HsqsMapperMap *map) {
	map->data = NULL;
	map->size = 0;
	return 0;
}
static int
hsqs_mapper_mmap_complete_cleanup(struct HsqsMapper *mapper) {
	int rv;

	rv = munmap(mapper->data.mc.data, mapper->size);
	return rv;
}

struct HsqsMapperImpl hsqs_mapper_impl_mmap_complete = {
		.init = hsqs_mapper_mmap_complete_init,
		.map = hsqs_mapper_mmap_complete_map,
		.unmap = hsqs_mapper_mmap_complete_unmap,
		.cleanup = hsqs_mapper_mmap_complete_cleanup,
};
