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

#define _DEFAULT_SOURCE

#include <sqsh_common_private.h>
#include <sqsh_error.h>
#include <sqsh_mapper_private.h>

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

static int
sqsh_mapper_file_init(
		struct SqshMapper *mapper, const void *input, size_t *size) {
	(void)size;
	int rv = 0;
	FILE *file;
	int fd;
	struct stat st = {0};

	file = fopen(input, "r");
	if (file == NULL) {
		rv = -errno;
		goto out;
	}

	fd = fileno(file);

	if (fstat(fd, &st) < 0) {
		fclose(file);
		rv = -errno;
		goto out;
	}

	// TODO: check for overflow.
	*size = (size_t)st.st_size;
	mapper->data.fm.file = file;

out:
	return rv;
}
static int
sqsh_mapping_file_map(struct SqshMapSlice *mapping) {
	int rv = 0;
	const size_t offset = mapping->offset;
	const size_t size = mapping->size;
	const struct SqshMapper *mapper = mapping->mapper;

	uint8_t *data = NULL;

	if (size != 0) {
		data = calloc(size, sizeof(uint8_t));
		if (data == NULL) {
			rv = -SQSH_ERROR_MALLOC_FAILED;
			goto out;
		}
		fseek(mapper->data.fm.file, (long)offset, SEEK_SET);
		if (fread(data, sizeof(uint8_t), size, mapper->data.fm.file) != size) {
			rv = -SQSH_ERROR_READ_FAILED;
			goto out;
		}
	}

	mapping->data = data;
out:
	if (rv < 0) {
		free(data);
	}
	return 0;
}

static int
sqsh_mapper_file_cleanup(struct SqshMapper *mapper) {
	fclose(mapper->data.fm.file);
	return 0;
}

static int
sqsh_mapping_file_unmap(struct SqshMapSlice *mapping) {
	free(mapping->data);
	return 0;
}
static const uint8_t *
sqsh_mapping_file_data(const struct SqshMapSlice *mapping) {
	const uint8_t *data = mapping->data;

	return data;
}

static const struct SqshMemoryMapperImpl impl = {
		/* 1 MiB */
		.block_size_hint = 1024 * 1024,   .init = sqsh_mapper_file_init,
		.map = sqsh_mapping_file_map,     .map_data = sqsh_mapping_file_data,
		.unmap = sqsh_mapping_file_unmap, .cleanup = sqsh_mapper_file_cleanup,
};
const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_file = &impl;
