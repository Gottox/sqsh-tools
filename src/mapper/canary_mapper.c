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
 * @file        : canary_mapper
 * @created     : Sunday Nov 21, 2021 16:01:03 CET
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
hsqs_mapper_canary_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	mapper->data.cn.data = input;
	mapper->data.cn.size = size;
	return 0;
}
static int
hsqs_mapper_canary_map(struct HsqsMapping *mapping, off_t offset, size_t size) {
	uint8_t *data = calloc(size, sizeof(uint8_t));

	memcpy(data, &mapping->mapper->data.cn.data[offset], size);
	mapping->data.cn.offset = offset;
	mapping->data.cn.data = data;
	mapping->data.cn.size = size;
	return 0;
}
static size_t
hsqs_mapper_canary_size(const struct HsqsMapper *mapper) {
	return mapper->data.cn.size;
}
static int
hsqs_mapper_canary_cleanup(struct HsqsMapper *mapper) {
	(void)mapper;
	return 0;
}
static int
hsqs_mapping_canary_unmap(struct HsqsMapping *mapping) {
	free(mapping->data.cn.data);
	mapping->data.cn.data = NULL;
	mapping->data.cn.size = 0;
	return 0;
}
static const uint8_t *
hsqs_mapping_canary_data(const struct HsqsMapping *mapping) {
	return mapping->data.cn.data;
}

static int
hsqs_mapping_canary_resize(struct HsqsMapping *mapping, size_t new_size) {
	int rv;
	uint64_t offset = mapping->data.cn.offset;
	struct HsqsMapper *mapper = mapping->mapper;

	rv = hsqs_mapping_unmap(mapping);
	if (rv < 0) {
		return rv;
	}
	return hsqs_mapper_map(mapping, mapper, offset, new_size);
}

static size_t
hsqs_mapping_canary_size(const struct HsqsMapping *mapping) {
	return mapping->data.cn.size;
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_canary = {
		.init = hsqs_mapper_canary_init,
		.mapping = hsqs_mapper_canary_map,
		.size = hsqs_mapper_canary_size,
		.cleanup = hsqs_mapper_canary_cleanup,
		.map_data = hsqs_mapping_canary_data,
		.map_resize = hsqs_mapping_canary_resize,
		.map_size = hsqs_mapping_canary_size,
		.unmap = hsqs_mapping_canary_unmap,
};
