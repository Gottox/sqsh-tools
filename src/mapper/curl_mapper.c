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
#include "mapper.h"
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
	size_t byte_size;
	struct HsqsMap *map = userdata;

	if (MULT_OVERFLOW(size, nmemb, &byte_size)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	rv = hsqs_buffer_append(&map->data.cl.buffer, ptr, byte_size);
	if (rv < 0) {
		goto out;
	}

	rv = byte_size;

out:
	return rv;
}

static CURL *
get_handle(struct HsqsMapper *mapper) {
	CURL *handle = mapper->data.cl.handle;
	curl_easy_reset(handle);
	curl_easy_setopt(handle, CURLOPT_URL, mapper->data.cl.url);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(handle, CURLOPT_FILETIME, 1L);
	return handle;
}

static int
hsqs_mapper_curl_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	int rv = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	mapper->data.cl.url = input;
	mapper->data.cl.handle = curl_easy_init();

	CURL *handle = get_handle(mapper);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
	rv = curl_easy_perform(handle);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_INIT;
		goto out;
	}

	curl_off_t content_length;
	rv = curl_easy_getinfo(
			handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_INIT;
		goto out;
	}
	mapper->data.cl.content_length = content_length;

	long file_time;
	rv = curl_easy_getinfo(handle, CURLINFO_FILETIME, &file_time);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_INIT;
		goto out;
	}
	mapper->data.cl.file_time = file_time;

out:
	return rv;
}
static int
hsqs_mapper_curl_map(
		struct HsqsMap *map, struct HsqsMapper *mapper, off_t offset,
		size_t size) {
	int rv = 0;

	map->data.cl.offset = offset;
	rv = hsqs_buffer_init(&map->data.cl.buffer, HSQS_COMPRESSION_NONE, 8192);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_map_resize(map, size);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}
static size_t
hsqs_mapper_curl_size(const struct HsqsMapper *mapper) {
	return mapper->data.cl.content_length;
}
static int
hsqs_mapper_curl_cleanup(struct HsqsMapper *mapper) {
	curl_easy_cleanup(mapper->data.cl.handle);
	return 0;
}
static int
hsqs_map_curl_unmap(struct HsqsMap *map) {
	hsqs_buffer_cleanup(&map->data.cl.buffer);
	return 0;
}
static const uint8_t *
hsqs_map_curl_data(const struct HsqsMap *map) {
	return hsqs_buffer_data(&map->data.cl.buffer);
}

static int
hsqs_map_curl_resize(struct HsqsMap *map, size_t new_size) {
	int rv = 0;
	char range_buffer[512] = {0};
	CURL *handle = get_handle(map->mapper);
	new_size = HSQS_PADDING(new_size, 512);
	size_t current_size = hsqs_map_size(map);
	uint64_t new_offset = map->data.cl.offset + current_size;
	uint64_t end_offset = new_offset + new_size - current_size - 1;
	long http_code = 0;

	if (new_size <= current_size) {
		return 0;
	}

	if (end_offset > map->mapper->data.cl.content_length - 1) {
		end_offset = map->mapper->data.cl.content_length - 1;
	}

	// TODO: check for negative values of offset
	rv = snprintf(
			range_buffer, sizeof(range_buffer), "%lu-%lu", new_offset,
			end_offset);
	if (rv >= sizeof(range_buffer)) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}
	curl_easy_setopt(handle, CURLOPT_RANGE, range_buffer);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, map);

	rv = curl_easy_perform(handle);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

	rv = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_code);

	if (http_code != 206 || rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

	long file_time;
	rv = curl_easy_getinfo(handle, CURLINFO_FILETIME, &file_time);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}
	if (map->mapper->data.cl.file_time != file_time) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

	rv = 0;

out:
	return rv;
}

static size_t
hsqs_map_curl_size(const struct HsqsMap *map) {
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
