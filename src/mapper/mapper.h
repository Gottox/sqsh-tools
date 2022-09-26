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
 * @file         mapper.h
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../utils.h"
#include "canary_mapper.h"
#include "mmap_full_mapper.h"
#include "mmap_mapper.h"
#include "static_mapper.h"
#ifdef CONFIG_CURL
#include "curl_mapper.h"
#endif

#ifndef MEMORY_MAPPER_H

#define MEMORY_MAPPER_H

extern struct SqshMemoryMapperImpl sqsh_mapper_impl_static;
extern struct SqshMemoryMapperImpl sqsh_mapper_impl_mmap_full;
extern struct SqshMemoryMapperImpl sqsh_mapper_impl_mmap;
extern struct SqshMemoryMapperImpl sqsh_mapper_impl_canary;
#ifdef CONFIG_CURL
extern struct SqshMemoryMapperImpl sqsh_mapper_impl_curl;
#endif

struct SqshMapper;

struct SqshMapping {
	struct SqshMapper *mapper;
	union {
		struct SqshMmapFullMap mc;
		struct SqshMmapMap mm;
		struct SqshStaticMap sm;
		struct SqshCanaryMap cn;
#ifdef CONFIG_CURL
		struct SqshCurlMap cl;
#endif
	} data;
};

struct SqshMemoryMapperImpl {
	int (*init)(struct SqshMapper *mapper, const void *input, size_t size);
	int (*mapping)(struct SqshMapping *map, off_t offset, size_t size);
	size_t (*size)(const struct SqshMapper *mapper);
	int (*cleanup)(struct SqshMapper *mapper);
	const uint8_t *(*map_data)(const struct SqshMapping *mapping);
	int (*map_resize)(struct SqshMapping *mapping, size_t new_size);
	size_t (*map_size)(const struct SqshMapping *mapping);
	int (*unmap)(struct SqshMapping *mapping);
};

struct SqshMapper {
	struct SqshMemoryMapperImpl *impl;
	union {
		struct SqshMmapFullMapper mc;
		struct SqshMmapMapper mm;
		struct SqshStaticMapper sm;
		struct SqshCanaryMapper cn;
#ifdef CONFIG_CURL
		struct SqshCurlMapper cl;
#endif
	} data;
};

int sqsh_mapper_init(
		struct SqshMapper *mapper, struct SqshMemoryMapperImpl *impl,
		const void *input, size_t size);
int sqsh_mapper_map(
		struct SqshMapping *mapping, struct SqshMapper *mapper,
		sqsh_index_t offset, size_t size);
size_t sqsh_mapper_size(const struct SqshMapper *mapper);
int sqsh_mapper_cleanup(struct SqshMapper *mapper);
size_t sqsh_mapping_size(struct SqshMapping *mapping);
int sqsh_mapping_resize(struct SqshMapping *mapping, size_t new_size);
const uint8_t *sqsh_mapping_data(const struct SqshMapping *mapping);
int sqsh_mapping_unmap(struct SqshMapping *mapping);

#endif /* end of include guard MEMORY_MAPPER_H */
