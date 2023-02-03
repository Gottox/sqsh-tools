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
 * @file         curl_mapper.c
 */

#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_mapper.h"
#include "../utils.h"

#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SUPERBLOCK_REQUEST_SIZE \
	(SQSH_SIZEOF_SUPERBLOCK + SQSH_SIZEOF_COMPRESSION_OPTIONS)
#define CONTENT_RANGE "Content-Range: "
#define CONTENT_RANGE_LENGTH (sizeof(CONTENT_RANGE) - 1)

static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	int rv = 0;
	size_t byte_size;
	struct SqshMapping *mapping = userdata;

	if (SQSH_MULT_OVERFLOW(size, nmemb, &byte_size)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	rv = sqsh_buffer_append(&mapping->data.cl.buffer, ptr, byte_size);
	if (rv < 0) {
		goto out;
	}

	rv = byte_size;

out:
	return rv;
}

static size_t
header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
	static const char *format =
			"Content-Range: bytes %" PRIu64 "-%" PRIu64 "/%" PRIu64 "\r\n%n";
	int rv = 0;
	uint64_t start = 0;
	uint64_t end = 0;
	uint64_t total = 0;
	size_t header_size = size * nitems;
	struct SqshMapping *mapping = (struct SqshMapping *)userdata;

	if (header_size < CONTENT_RANGE_LENGTH) {
		return header_size;
	}
	if (strncmp(buffer, CONTENT_RANGE, CONTENT_RANGE_LENGTH) != 0) {
		return header_size;
	}

	char *header = sqsh_memdup(buffer, header_size);
	if (header == NULL) {
		return 0;
	}
	rv = 0;
	sscanf(header, format, &start, &end, &total, &rv);

	if ((size_t)rv != header_size) {
		free(header);
		return 0;
	}

	mapping->data.cl.total_size = total;

	free(header);
	return header_size;
}

static CURL *
get_handle(struct SqshMapping *mapping) {
	CURL *handle = mapping->mapper->data.cl.handle;
	pthread_mutex_lock(&mapping->mapper->data.cl.handle_lock);
	curl_easy_reset(handle);
	curl_easy_setopt(handle, CURLOPT_URL, mapping->mapper->data.cl.url);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(handle, CURLOPT_FILETIME, 1L);
	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
	curl_easy_setopt(handle, CURLOPT_HEADERDATA, mapping);
	return handle;
}

static void
release_handle(struct SqshMapper *mapper, CURL *handle) {
	if (handle == mapper->data.cl.handle) {
		pthread_mutex_unlock(&mapper->data.cl.handle_lock);
	}
}

static int
sqsh_mapper_curl_init(
		struct SqshMapper *mapper, const void *input, size_t size) {
	(void)size;
	int rv = 0;
	struct SqshMapping mapping = {0};
	curl_global_init(CURL_GLOBAL_ALL);

	mapper->data.cl.url = input;
	mapper->data.cl.handle = curl_easy_init();
	mapper->data.cl.expected_size = UINT64_MAX;
	mapper->data.cl.expected_time = UINT64_MAX;

	rv = pthread_mutex_init(&mapper->data.cl.handle_lock, NULL);
	if (rv != 0) {
		rv = -SQSH_ERROR_MAPPER_INIT;
		goto out;
	}

	rv = sqsh_mapper_map(&mapping, mapper, 0, SUPERBLOCK_REQUEST_SIZE);
	if (rv < 0) {
		goto out;
	}
	mapper->data.cl.expected_size = mapping.data.cl.total_size;
	mapper->data.cl.expected_time = mapping.data.cl.file_time;
	sqsh_mapping_unmap(&mapping);

out:
	return rv;
}

static int
sqsh_mapper_curl_map(
		struct SqshMapping *mapping, sqsh_index_t offset, size_t size) {
	int rv = 0;

	mapping->data.cl.offset = offset;
	rv = sqsh_buffer_init(&mapping->data.cl.buffer);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_mapping_resize(mapping, size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		sqsh_mapping_unmap(mapping);
	}
	return rv;
}
static size_t
sqsh_mapper_curl_size(const struct SqshMapper *mapper) {
	return mapper->data.cl.expected_size;
}

static int
sqsh_mapper_curl_cleanup(struct SqshMapper *mapper) {
	pthread_mutex_destroy(&mapper->data.cl.handle_lock);
	curl_easy_cleanup(mapper->data.cl.handle);
	return 0;
}

static int
sqsh_mapping_curl_unmap(struct SqshMapping *mapping) {
	sqsh_buffer_cleanup(&mapping->data.cl.buffer);
	return 0;
}
static const uint8_t *
sqsh_mapping_curl_data(const struct SqshMapping *mapping) {
	return sqsh_buffer_data(&mapping->data.cl.buffer);
}

static int
sqsh_mapping_curl_resize(struct SqshMapping *mapping, size_t new_size) {
	int rv = 0;
	char range_buffer[512] = {0};
	CURL *handle = get_handle(mapping);
	new_size = SQSH_PADDING(new_size, 512);
	size_t current_size = sqsh_mapping_size(mapping);
	uint64_t new_offset = mapping->data.cl.offset + current_size;
	uint64_t end_offset = new_offset + new_size - current_size - 1;
	long http_code = 0;
	uint64_t expected_size = mapping->mapper->data.cl.expected_size;
	uint64_t expected_time = mapping->mapper->data.cl.expected_time;

	if (new_size <= current_size) {
		return 0;
	}

	if (end_offset > mapping->mapper->data.cl.expected_size - 1) {
		end_offset = mapping->mapper->data.cl.expected_size - 1;
	}

	// TODO: check for negative values of offset
	rv = snprintf(
			range_buffer, sizeof(range_buffer), "%" PRIu64 "-%" PRIu64,
			new_offset, end_offset);
	if (rv >= (int)sizeof(range_buffer)) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}
	curl_easy_setopt(handle, CURLOPT_RANGE, range_buffer);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, mapping);

	rv = curl_easy_perform(handle);
	if (rv != CURLE_OK) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	rv = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_code);

	if (http_code != 206 || rv != CURLE_OK) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	uint64_t file_time;
	rv = curl_easy_getinfo(handle, CURLINFO_FILETIME, &file_time);
	if (rv != CURLE_OK) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}
	rv = 0;
	mapping->data.cl.file_time = file_time;

	if (file_time == UINT64_MAX) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_time != UINT64_MAX && file_time != expected_time) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_size != UINT64_MAX &&
		mapping->data.cl.total_size != expected_size) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

out:
	release_handle(mapping->mapper, handle);
	return rv;
}

static size_t
sqsh_mapping_curl_size(const struct SqshMapping *mapping) {
	return sqsh_buffer_size(&mapping->data.cl.buffer);
}

struct SqshMemoryMapperImpl sqsh_mapper_impl_curl = {
		.init = sqsh_mapper_curl_init,
		.mapping = sqsh_mapper_curl_map,
		.size = sqsh_mapper_curl_size,
		.cleanup = sqsh_mapper_curl_cleanup,
		.map_data = sqsh_mapping_curl_data,
		.map_resize = sqsh_mapping_curl_resize,
		.map_size = sqsh_mapping_curl_size,
		.unmap = sqsh_mapping_curl_unmap,
};
