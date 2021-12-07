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

#include "mapper.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int
hsqs_mapper_static_mem_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	mapper->data.sm.data = input;
	mapper->data.sm.size = size;
	return 0;
}
static int
hsqs_mapper_static_mem_map(
		struct HsqsMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size) {
	map->data.sm.data = &mapper->data.sm.data[offset];
	map->data.sm.size = size;
	return 0;
}
static size_t
hsqs_mapper_static_mem_size(const struct HsqsMapper *mapper) {
	return mapper->data.sm.size;
}
static int
hsqs_mapper_static_mem_cleanup(struct HsqsMapper *mapper) {
	return 0;
}
static int
hsqs_map_static_mem_unmap(struct HsqsMap *map) {
	map->data.sm.data = NULL;
	map->data.sm.size = 0;
	return 0;
}
static const uint8_t *
hsqs_map_static_mem_data(const struct HsqsMap *map) {
	return map->data.sm.data;
}

static int
hsqs_map_static_mem_resize(struct HsqsMap *map, size_t new_size) {
	return map->data.sm.size;
}

static size_t
hsqs_map_static_mem_size(const struct HsqsMap *map) {
	return map->data.sm.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_static = {
		.init = hsqs_mapper_static_mem_init,
		.map = hsqs_mapper_static_mem_map,
		.size = hsqs_mapper_static_mem_size,
		.cleanup = hsqs_mapper_static_mem_cleanup,
		.map_data = hsqs_map_static_mem_data,
		.map_resize = hsqs_map_static_mem_resize,
		.map_size = hsqs_map_static_mem_size,
		.unmap = hsqs_map_static_mem_unmap,
};
