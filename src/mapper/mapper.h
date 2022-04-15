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
 * @file        : mapper
 * @created     : Sunday Nov 21, 2021 12:17:35 CET
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "canary_mapper.h"
#include "curl_mapper.h"
#include "mmap_full_mapper.h"
#include "mmap_mapper.h"
#include "static_mapper.h"

#ifndef MEMORY_MAPPER_H

#define MEMORY_MAPPER_H

struct HsqsMapper;

struct HsqsMapping {
	struct HsqsMapper *mapper;
	union {
		struct HsqsMmapFullMap mc;
		struct HsqsMmapMap mm;
		struct HsqsStaticMap sm;
		struct HsqsCanaryMap cn;
		struct HsqsCurlMap cl;
	} data;
};

struct HsqsMemoryMapperImpl {
	int (*init)(struct HsqsMapper *mapper, const void *input, size_t size);
	int (*mapping)(struct HsqsMapping *map, off_t offset, size_t size);
	size_t (*size)(const struct HsqsMapper *mapper);
	int (*cleanup)(struct HsqsMapper *mapper);
	const uint8_t *(*map_data)(const struct HsqsMapping *mapping);
	int (*map_resize)(struct HsqsMapping *mapping, size_t new_size);
	size_t (*map_size)(const struct HsqsMapping *mapping);
	int (*unmap)(struct HsqsMapping *mapping);
};

struct HsqsMapper {
	struct HsqsMemoryMapperImpl *impl;
	union {
		struct HsqsMmapFullMapper mc;
		struct HsqsMmapMapper mm;
		struct HsqsStaticMapper sm;
		struct HsqsCanaryMapper cn;
		struct HsqsCurlMapper cl;
	} data;
};

int hsqs_mapper_init_mmap(struct HsqsMapper *mapper, const char *path);
int hsqs_mapper_init_static(
		struct HsqsMapper *mapper, const uint8_t *input, size_t size);
int hsqs_mapper_map(
		struct HsqsMapping *mapping, struct HsqsMapper *mapper,
		hsqs_index_t offset, size_t size);
size_t hsqs_mapper_size(const struct HsqsMapper *mapper);
int hsqs_mapper_cleanup(struct HsqsMapper *mapper);
size_t hsqs_mapping_size(struct HsqsMapping *mapping);
int hsqs_mapping_resize(struct HsqsMapping *mapping, size_t new_size);
const uint8_t *hsqs_mapping_data(const struct HsqsMapping *mapping);
int hsqs_mapping_unmap(struct HsqsMapping *mapping);

#endif /* end of include guard MEMORY_MAPPER_H */
