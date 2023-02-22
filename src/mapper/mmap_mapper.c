/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @author       Enno Boland (mail@eboland.de)
 * @file         mmap_mapper.c
 */

#define _GNU_SOURCE

#include "../../include/sqsh_mapper_private.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int
sqsh_mapper_mmap_init(
		struct SqshMapper *mapper, const void *input, size_t *size) {
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
	*size = st.st_size;
	mapper->data.mm.fd = fd;
	mapper->data.mm.page_size = sysconf(_SC_PAGESIZE);
	fd = -1;

out:
	if (fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
sqsh_mapper_mmap_map(
		struct SqshMapping *mapping, sqsh_index_t offset, size_t size) {
	struct SqshMapper *mapper = mapping->mapper;
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
	mapping->data.mm.page_offset = page_offset;
	return 0;
}

static int
sqsh_mapper_mmap_cleanup(struct SqshMapper *mapper) {
	close(mapper->data.mm.fd);
	return 0;
}

static int
sqsh_mapping_mmap_unmap(struct SqshMapping *mapping) {
	int rv;
	rv = munmap(
			mapping->data.mm.data,
			sqsh__mapper_size(mapping->mapper) + mapping->data.mm.page_offset);

	mapping->data.mm.data = NULL;
	mapping->data.mm.page_offset = 0;
	return rv;
}
static const uint8_t *
sqsh_mapping_mmap_data(const struct SqshMapping *mapping) {
	return &mapping->data.mm.data[mapping->data.mm.page_offset];
}

static const struct SqshMemoryMapperImpl impl = {
#if UINTPTR_MAX >= UINT64_MAX
		// 1 GiB
		.block_size_hint = 1 * 1024 * 1024 * 1024,
#else
		// 100 MiB
		.block_size_hint = 100 * 1024 * 1024,
#endif
		.init = sqsh_mapper_mmap_init,
		.mapping = sqsh_mapper_mmap_map,
		.cleanup = sqsh_mapper_mmap_cleanup,
		.map_data = sqsh_mapping_mmap_data,
		.unmap = sqsh_mapping_mmap_unmap,
};
const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_mmap = &impl;
