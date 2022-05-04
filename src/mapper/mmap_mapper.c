/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
	(void)size;
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
hsqs_mapper_mmap_map(struct HsqsMapping *mapping, off_t offset, size_t size) {
	struct HsqsMapper *mapper = mapping->mapper;
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

	mapping->data.mm.data = file_map;
	mapping->data.mm.size = size;
	mapping->data.mm.offset = offset;
	mapping->data.mm.page_offset = page_offset;
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
hsqs_mapping_mmap_unmap(struct HsqsMapping *mapping) {
	int rv;
	rv =
			munmap(mapping->data.mm.data,
				   mapping->data.mm.size + mapping->data.mm.page_offset);

	mapping->data.mm.data = NULL;
	mapping->data.mm.size = 0;
	mapping->data.mm.page_offset = 0;
	mapping->data.mm.offset = 0;
	return rv;
}
static const uint8_t *
hsqs_mapping_mmap_data(const struct HsqsMapping *mapping) {
	return &mapping->data.mm.data[mapping->data.mm.page_offset];
}

int
hsqs_mapping_mmap_resize(struct HsqsMapping *mapping, size_t new_size) {
#ifdef _GNU_SOURCE
	uint8_t *data = mapping->data.mm.data;
	size_t size = mapping->data.mm.size;
	size_t page_offset = mapping->data.mm.page_offset;

	data = mremap(
			data, page_offset + size, page_offset + new_size, MREMAP_MAYMOVE);

	if (data == MAP_FAILED) {
		return -errno;
	}

	mapping->data.mm.size = new_size;
	mapping->data.mm.data = data;

	return 0;
#else
	int rv;
	uint64_t offset = mapping->data.mm.offset;
	struct HsqsMemoryMapper *mapper = mapping->mapper;

	rv = hsqs_mapping_unmap(mapping);
	if (rv < 0) {
		return rv;
	}
	return hsqs_mapper_map(mapping, mapper, offset, new_size);
#endif
}
static size_t
hsqs_mapping_mmap_size(const struct HsqsMapping *mapping) {
	return mapping->data.mm.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_mmap = {
		.init = hsqs_mapper_mmap_init,
		.mapping = hsqs_mapper_mmap_map,
		.size = hsqs_mapper_mmap_size,
		.cleanup = hsqs_mapper_mmap_cleanup,
		.map_data = hsqs_mapping_mmap_data,
		.map_size = hsqs_mapping_mmap_size,
		.map_resize = hsqs_mapping_mmap_resize,
		.unmap = hsqs_mapping_mmap_unmap,
};
