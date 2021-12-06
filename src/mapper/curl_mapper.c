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
 * @file        : curl_mapper
 * @created     : Monday Dec 06, 2021 17:51:32 CET
 */

#include "../error.h"
#include "memory_mapper.h"
#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	int rv = 0;
	struct HsqsMemoryMap *map = userdata;

	// TODO integer overflow
	rv = hsqs_buffer_append(&map->data.cl.buffer, ptr, size * nmemb, false);
	if (rv < 0) {
		goto out;
	}

	rv = size * nmemb;

out:
	return rv;
}

static int
hsqs_mapper_curl_init(
		struct HsqsMemoryMapper *mapper, const void *input, size_t size) {
	int rv = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	mapper->data.cl.url = input;

	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, mapper->data.cl.url);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
	rv = curl_easy_perform(handle);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_TODO;
		goto out;
	}

	curl_off_t content_length;
	rv = curl_easy_getinfo(
			handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_TODO;
		goto out;
	}
	mapper->data.cl.content_length = content_length;

out:
	return rv;
}
static int
hsqs_mapper_curl_map(
		struct HsqsMemoryMap *map, struct HsqsMemoryMapper *mapper,
		off_t offset, size_t size) {
	char range_buffer[512] = {0};
	int rv = 0;
	CURL *handle = curl_easy_init();
	map->data.cl.offset = offset;
	rv = hsqs_buffer_init(&map->data.cl.buffer, HSQS_COMPRESSION_NONE, 8192);
	if (rv < 0) {
		goto out;
	}

	curl_easy_setopt(handle, CURLOPT_URL, mapper->data.cl.url);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, map);
	// TODO: check for negative values of offset
	rv = snprintf(
			range_buffer, sizeof(range_buffer), "%lu-%lu", offset,
			offset + size - 1);
	if (rv >= sizeof(range_buffer)) {
		rv = -HSQS_ERROR_TODO;
		goto out;
	}
	curl_easy_setopt(handle, CURLOPT_RANGE, range_buffer);

	rv = curl_easy_perform(handle);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_TODO;
	}

out:
	curl_easy_cleanup(handle);
	return rv;
}
static size_t
hsqs_mapper_curl_size(const struct HsqsMemoryMapper *mapper) {
	return mapper->data.cl.content_length;
}
static int
hsqs_mapper_curl_cleanup(struct HsqsMemoryMapper *mapper) {
	return 0;
}
static int
hsqs_map_curl_unmap(struct HsqsMemoryMap *map) {
	hsqs_buffer_cleanup(&map->data.cl.buffer);

	return 0;
}
static const uint8_t *
hsqs_map_curl_data(const struct HsqsMemoryMap *map) {
	return hsqs_buffer_data(&map->data.cl.buffer);
}

static int
hsqs_map_curl_resize(struct HsqsMemoryMap *map, size_t new_size) {
	int rv;
	uint64_t offset = map->data.cl.offset;
	struct HsqsMemoryMapper *mapper = map->mapper;

	rv = hsqs_map_unmap(map);
	if (rv < 0) {
		return rv;
	}
	return hsqs_mapper_map(map, mapper, offset, new_size);
}

static size_t
hsqs_map_curl_size(const struct HsqsMemoryMap *map) {
	return hsqs_buffer_size(&map->data.cl.buffer);
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_curl = {
		.init = hsqs_mapper_curl_init,
		.map = hsqs_mapper_curl_map,
		.size = hsqs_mapper_curl_size,
		.cleanup = hsqs_mapper_curl_cleanup,
		.map_data = hsqs_map_curl_data,
		.map_resize = hsqs_map_curl_resize,
		.map_size = hsqs_map_curl_size,
		.unmap = hsqs_map_curl_unmap,
};
