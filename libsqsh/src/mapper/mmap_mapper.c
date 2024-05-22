/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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

#define _LARGEFILE64_SOURCE

#include <sqsh_common_private.h>
#include <sqsh_error.h>
#include <sqsh_mapper_private.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define TO_PTR(x) ((void *)(uintptr_t)(x))
#define FROM_PTR(x) ((int)(uintptr_t)(x))

static long page_size = 0;
static pthread_once_t page_size_initialized = PTHREAD_ONCE_INIT;

static void
init_page_size(void) {
	page_size = sysconf(_SC_PAGESIZE);
}

static int
sqsh_mapper_mmap_init(
		struct SqshMapper *mapper, const void *input, size_t *size) {
	(void)size;
	(void)mapper;
	int rv = 0;
	int fd = -1;
	off64_t pos = 0;

	rv = pthread_once(&page_size_initialized, init_page_size);
	if (rv != 0) {
		return -SQSH_ERROR_INTERNAL;
	}

	fd = open(input, 0);
	if (fd < 0) {
		rv = -errno;
		goto out;
	}

	pos = lseek64(fd, 0, SEEK_END);
	if (pos < 0) {
		rv = -errno;
		goto out;
	}
	*size = (size_t)pos;

	sqsh_mapper_set_user_data(mapper, TO_PTR(fd));

out:
	if (rv < 0 && fd >= 0) {
		close(fd);
	}
	return rv;
}
static int
sqsh_mapping_mmap_map(
		const struct SqshMapper *mapper, sqsh_index_t offset, size_t size,
		uint8_t **data) {
	const int fd = FROM_PTR(sqsh_mapper_user_data(mapper));

	const off_t mmap_offset = (off_t)offset % page_size;
	const size_t mmap_size = (size_t)mmap_offset + size;

	uint8_t *file_map = NULL;

	if (size != 0) {
		file_map =
				mmap(NULL, mmap_size, PROT_READ, MAP_PRIVATE, fd,
					 (off_t)offset - mmap_offset);
		if (file_map == MAP_FAILED) {
			return -errno;
		}
	}

	*data = &file_map[mmap_offset];
	return 0;
}

static int
sqsh_mapper_mmap_cleanup(struct SqshMapper *mapper) {
	const int fd = FROM_PTR(sqsh_mapper_user_data(mapper));
	close(fd);
	return 0;
}

static int
sqsh_mapping_mmap_unmap(const struct SqshMapper *mapper, uint8_t *data, size_t size) {
	(void)mapper;
	const uintptr_t offset = (uintptr_t)data % (uintptr_t)page_size;

	return munmap(data - offset, size + offset);
}

static const struct SqshMemoryMapperImpl impl = {
#if UINTPTR_MAX >= UINT64_MAX
		/* 1 GiB */
		.block_size_hint = 1 * 1024 * 1024 * 1024,
#else
		/* 100 MiB */
		.block_size_hint = 100 * 1024 * 1024,
#endif
		.init = sqsh_mapper_mmap_init,
		.map = sqsh_mapping_mmap_map,
		.unmap = sqsh_mapping_mmap_unmap,
		.cleanup = sqsh_mapper_mmap_cleanup,
};
const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_mmap = &impl;
