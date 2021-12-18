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
 * @file        : mmap_mapper
 * @created     : Sunday Nov 21, 2021 16:01:03 CET
 */

#define _GNU_SOURCE
#include "../utils.h"
#include "mapper.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int
hsqs_mapper_mmap_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
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
	mapper->data.mm.page_size = sysconf(_SC_PAGESIZE);
	fd = -1;

out:
	if (fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
hsqs_mapper_mmap_map(struct HsqsMap *map, off_t offset, size_t size) {
	struct HsqsMapper *mapper = map->mapper;
	uint8_t *file_map = NULL;
	size_t page_offset = offset % mapper->data.mm.page_size;

	if (size != 0) {
		file_map =
				mmap(NULL, size + page_offset, PROT_READ, MAP_PRIVATE,
					 mapper->data.mm.fd, offset - page_offset);
		if (file_map == MAP_FAILED) {
			return -errno;
		}
	}

	map->data.mm.data = file_map;
	map->data.mm.size = size;
	map->data.mm.offset = offset;
	map->data.mm.page_offset = page_offset;
	return 0;
}
static int
hsqs_mapper_mmap_cleanup(struct HsqsMapper *mapper) {
	close(mapper->data.mm.fd);
	return 0;
}
static size_t
hsqs_mapper_mmap_size(const struct HsqsMapper *mapper) {
	return mapper->data.mm.size;
}

static int
hsqs_map_mmap_unmap(struct HsqsMap *map) {
	int rv;
	rv = munmap(
			map->data.mm.data, map->data.mm.size + map->data.mm.page_offset);

	map->data.mm.data = NULL;
	map->data.mm.size = 0;
	map->data.mm.page_offset = 0;
	map->data.mm.offset = 0;
	return rv;
}
static const uint8_t *
hsqs_map_mmap_data(const struct HsqsMap *map) {
	return &map->data.mm.data[map->data.mm.page_offset];
}

int
hsqs_map_mmap_resize(struct HsqsMap *map, size_t new_size) {
#ifdef _GNU_SOURCE
	uint8_t *data = map->data.mm.data;
	size_t size = map->data.mm.size;
	size_t page_offset = map->data.mm.page_offset;

	data = mremap(
			data, page_offset + size, page_offset + new_size, MREMAP_MAYMOVE);

	if (data == MAP_FAILED) {
		return -errno;
	}

	map->data.mm.size = new_size;
	map->data.mm.data = data;

	return 0;
#else
	int rv;
	uint64_t offset = map->data.mm.offset;
	struct HsqsMemoryMapper *mapper = map->mapper;

	rv = hsqs_map_unmap(map);
	if (rv < 0) {
		return rv;
	}
	return hsqs_mapper_map(map, mapper, offset, new_size);
#endif
}
static size_t
hsqs_map_mmap_size(const struct HsqsMap *map) {
	return map->data.mm.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap = {
		.init = hsqs_mapper_mmap_init,
		.map = hsqs_mapper_mmap_map,
		.size = hsqs_mapper_mmap_size,
		.cleanup = hsqs_mapper_mmap_cleanup,
		.map_data = hsqs_map_mmap_data,
		.map_size = hsqs_map_mmap_size,
		.map_resize = hsqs_map_mmap_resize,
		.unmap = hsqs_map_mmap_unmap,
};
