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

#include <errno.h>
#include <fcntl.h>
#include <sqsh_mapper.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int
sqsh_mapper_static_mem_init(
		struct SqshMapper *mapper, const void *input, size_t size) {
	mapper->data.sm.data = input;
	mapper->data.sm.size = size;
	return 0;
}
static int
sqsh_mapper_static_mem_map(
		struct SqshMapping *mapping, sqsh_index_t offset, size_t size) {
	mapping->data.sm.data = &mapping->mapper->data.sm.data[offset];
	mapping->data.sm.size = size;
	return 0;
}
static size_t
sqsh_mapper_static_mem_size(const struct SqshMapper *mapper) {
	return mapper->data.sm.size;
}
static int
sqsh_mapper_static_mem_cleanup(struct SqshMapper *mapper) {
	(void)mapper;
	return 0;
}
static int
sqsh_mapping_static_mem_unmap(struct SqshMapping *mapping) {
	mapping->data.sm.data = NULL;
	mapping->data.sm.size = 0;
	return 0;
}
static const uint8_t *
sqsh_mapping_static_mem_data(const struct SqshMapping *mapping) {
	return mapping->data.sm.data;
}

static int
sqsh_mapping_static_mem_resize(struct SqshMapping *mapping, size_t new_size) {
	mapping->data.sm.size = new_size;
	return 0;
}

static size_t
sqsh_mapping_static_mem_size(const struct SqshMapping *mapping) {
	return mapping->data.sm.size;
}

struct SqshMemoryMapperImpl sqsh_mapper_impl_static = {
		.init = sqsh_mapper_static_mem_init,
		.mapping = sqsh_mapper_static_mem_map,
		.size = sqsh_mapper_static_mem_size,
		.cleanup = sqsh_mapper_static_mem_cleanup,
		.map_data = sqsh_mapping_static_mem_data,
		.map_resize = sqsh_mapping_static_mem_resize,
		.map_size = sqsh_mapping_static_mem_size,
		.unmap = sqsh_mapping_static_mem_unmap,
};
