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
 * @file        : mmap_complete
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
hsqs_mapper_mmap_complete_init(
		struct HsqsMemoryMapper *mapper, const void *input, size_t size) {
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

	file_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_map == MAP_FAILED) {
		rv = -errno;
	}
	mapper->data.mc.data = file_map;
	mapper->data.mc.size = st.st_size;

out:
	if (fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
hsqs_mapper_mmap_complete_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size) {
	map->data.mc.data = &mapper->data.mc.data[offset];
	map->data.mc.size = size;
	return 0;
}
static int
hsqs_mapper_mmap_complete_cleanup(struct HsqsMemoryMapper *mapper) {
	int rv;
	size_t size = hsqs_mapper_size(mapper);

	rv = munmap(mapper->data.mc.data, size);
	return rv;
}
static size_t
hsqs_mapper_mmap_complete_size(const struct HsqsMemoryMapper *mapper) {
	return mapper->data.mc.size;
}

static int
hsqs_map_mmap_complete_unmap(struct HsqsMemoryMap *map) {
	map->data.mc.data = NULL;
	map->data.mc.size = 0;
	return 0;
}
static const uint8_t *
hsqs_map_mmap_complete_data(const struct HsqsMemoryMap *map) {
	return map->data.mc.data;
}
static size_t
hsqs_map_mmap_complete_size(const struct HsqsMemoryMap *map) {
	return map->data.mc.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap_complete = {
		.init = hsqs_mapper_mmap_complete_init,
		.map = hsqs_mapper_mmap_complete_map,
		.size = hsqs_mapper_mmap_complete_size,
		.cleanup = hsqs_mapper_mmap_complete_cleanup,
		.map_data = hsqs_map_mmap_complete_data,
		.map_size = hsqs_map_mmap_complete_size,
		.unmap = hsqs_map_mmap_complete_unmap,
};
