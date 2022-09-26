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
 * @author       Enno Boland (mail@eboland.de)
 * @file         canary_mapper.c
 */

#include "mapper.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int
sqsh_mapper_canary_init(
		struct SqshMapper *mapper, const void *input, size_t size) {
	mapper->data.cn.data = input;
	mapper->data.cn.size = size;
	return 0;
}
static int
sqsh_mapper_canary_map(struct SqshMapping *mapping, off_t offset, size_t size) {
	uint8_t *data = calloc(size, sizeof(uint8_t));

	memcpy(data, &mapping->mapper->data.cn.data[offset], size);
	mapping->data.cn.offset = offset;
	mapping->data.cn.data = data;
	mapping->data.cn.size = size;
	return 0;
}
static size_t
sqsh_mapper_canary_size(const struct SqshMapper *mapper) {
	return mapper->data.cn.size;
}
static int
sqsh_mapper_canary_cleanup(struct SqshMapper *mapper) {
	(void)mapper;
	return 0;
}
static int
sqsh_mapping_canary_unmap(struct SqshMapping *mapping) {
	free(mapping->data.cn.data);
	mapping->data.cn.data = NULL;
	mapping->data.cn.size = 0;
	return 0;
}
static const uint8_t *
sqsh_mapping_canary_data(const struct SqshMapping *mapping) {
	return mapping->data.cn.data;
}

static int
sqsh_mapping_canary_resize(struct SqshMapping *mapping, size_t new_size) {
	int rv;
	uint64_t offset = mapping->data.cn.offset;
	struct SqshMapper *mapper = mapping->mapper;

	rv = sqsh_mapping_unmap(mapping);
	if (rv < 0) {
		return rv;
	}
	return sqsh_mapper_map(mapping, mapper, offset, new_size);
}

static size_t
sqsh_mapping_canary_size(const struct SqshMapping *mapping) {
	return mapping->data.cn.size;
}

struct SqshMemoryMapperImpl sqsh_mapper_impl_canary = {
		.init = sqsh_mapper_canary_init,
		.mapping = sqsh_mapper_canary_map,
		.size = sqsh_mapper_canary_size,
		.cleanup = sqsh_mapper_canary_cleanup,
		.map_data = sqsh_mapping_canary_data,
		.map_resize = sqsh_mapping_canary_resize,
		.map_size = sqsh_mapping_canary_size,
		.unmap = sqsh_mapping_canary_unmap,
};
