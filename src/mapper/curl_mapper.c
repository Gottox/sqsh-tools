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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : curl_mapper
 * @created     : Monday Dec 06, 2021 17:51:32 CET
 */

#include "../data/compression_options.h"
#include "../data/superblock.h"
#include "../error.h"
#include "inttypes.h"
#include "mapper.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SUPERBLOCK_REQUEST_SIZE \
	(HSQS_SIZEOF_SUPERBLOCK + HSQS_SIZEOF_COMPRESSION_OPTIONS)
#define CONTENT_RANGE "Content-Range: "
#define CONTENT_RANGE_LENGTH (sizeof(CONTENT_RANGE) - 1)

static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	int rv = 0;
	size_t byte_size;
	struct HsqsMapping *mapping = userdata;

	if (MULT_OVERFLOW(size, nmemb, &byte_size)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	rv = hsqs_buffer_append(mapping->data.cl.buffer, ptr, byte_size);
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
	struct HsqsMapping *mapping = (struct HsqsMapping *)userdata;

	if (header_size < CONTENT_RANGE_LENGTH) {
		return header_size;
	}
	if (strncmp(buffer, CONTENT_RANGE, CONTENT_RANGE_LENGTH) != 0) {
		return header_size;
	}

	char *header = hsqs_memdup(buffer, header_size);
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
get_handle(struct HsqsMapping *mapping) {
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
release_handle(struct HsqsMapper *mapper, CURL *handle) {
	if (handle == mapper->data.cl.handle) {
		pthread_mutex_unlock(&mapper->data.cl.handle_lock);
	}
}

static int
hsqs_mapper_curl_init(
		struct HsqsMapper *mapper, const void *input, size_t size) {
	(void)size;
	int rv = 0;
	struct HsqsMapping mapping = {0};
	curl_global_init(CURL_GLOBAL_ALL);

	mapper->data.cl.url = input;
	mapper->data.cl.handle = curl_easy_init();
	mapper->data.cl.expected_size = UINT64_MAX;
	mapper->data.cl.expected_time = UINT64_MAX;

	rv = hsqs_lru_hashmap_init(&mapper->data.cl.cache, 16);
	if (rv < 0) {
		goto out;
	}

	rv = pthread_mutex_init(&mapper->data.cl.handle_lock, NULL);
	if (rv != 0) {
		rv = -HSQS_ERROR_MAPPER_INIT;
		goto out;
	}

	rv = hsqs_mapper_map(&mapping, mapper, 0, SUPERBLOCK_REQUEST_SIZE);
	if (rv < 0) {
		goto out;
	}
	mapper->data.cl.expected_size = mapping.data.cl.total_size;
	mapper->data.cl.expected_time = mapping.data.cl.file_time;
	hsqs_mapping_unmap(&mapping);

out:
	return rv;
}

static int
buffer_dtor(void *data) {
	struct HsqsBuffer *buffer = data;
	hsqs_buffer_cleanup(buffer);
	return 0;
}

static int
hsqs_mapper_curl_map(struct HsqsMapping *mapping, off_t offset, size_t size) {
	int rv = 0;
	struct HsqsRefCount *buffer_ref;
	struct HsqsBuffer *buffer;

	mapping->data.cl.offset = offset;
	buffer_ref = hsqs_lru_hashmap_get(&mapping->mapper->data.cl.cache, offset);
	if (buffer_ref == NULL) {
		rv = hsqs_ref_count_new(
				&buffer_ref, sizeof(struct HsqsBuffer), buffer_dtor);
		if (rv < 0) {
			goto out;
		}
		rv = hsqs_lru_hashmap_put(
				&mapping->mapper->data.cl.cache, offset, buffer_ref);
		if (rv < 0) {
			goto out;
		}
		buffer = hsqs_ref_count_retain(buffer_ref);
		rv = hsqs_buffer_init(buffer, HSQS_COMPRESSION_NONE, 8192);
		if (rv < 0) {
			goto out;
		}
	} else {
		buffer = hsqs_ref_count_retain(buffer_ref);
	}
	mapping->data.cl.buffer_ref = buffer_ref;
	mapping->data.cl.buffer = buffer;

	rv = hsqs_mapping_resize(mapping, size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		hsqs_mapping_unmap(mapping);
	}
	return rv;
}
static size_t
hsqs_mapper_curl_size(const struct HsqsMapper *mapper) {
	return mapper->data.cl.expected_size;
}
static int
hsqs_mapper_curl_cleanup(struct HsqsMapper *mapper) {
	hsqs_lru_hashmap_cleanup(&mapper->data.cl.cache);
	pthread_mutex_destroy(&mapper->data.cl.handle_lock);
	curl_easy_cleanup(mapper->data.cl.handle);
	return 0;
}
static int
hsqs_mapping_curl_unmap(struct HsqsMapping *mapping) {
	hsqs_ref_count_release(mapping->data.cl.buffer_ref);
	mapping->data.cl.buffer_ref = NULL;
	return 0;
}
static const uint8_t *
hsqs_mapping_curl_data(const struct HsqsMapping *mapping) {
	return hsqs_buffer_data(mapping->data.cl.buffer);
}

static int
hsqs_mapping_curl_resize(struct HsqsMapping *mapping, size_t new_size) {
	int rv = 0;
	char range_buffer[512] = {0};
	CURL *handle = get_handle(mapping);
	new_size = HSQS_PADDING(new_size, 512);
	size_t current_size = hsqs_mapping_size(mapping);
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
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}
	curl_easy_setopt(handle, CURLOPT_RANGE, range_buffer);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, mapping);

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

	uint64_t file_time;
	rv = curl_easy_getinfo(handle, CURLINFO_FILETIME, &file_time);
	if (rv != CURLE_OK) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}
	rv = 0;
	mapping->data.cl.file_time = file_time;

	if (file_time == UINT64_MAX) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_time != UINT64_MAX && file_time != expected_time) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_size != UINT64_MAX &&
		mapping->data.cl.total_size != expected_size) {
		rv = -HSQS_ERROR_MAPPER_MAP;
		goto out;
	}

out:
	release_handle(mapping->mapper, handle);
	return rv;
}

static size_t
hsqs_mapping_curl_size(const struct HsqsMapping *mapping) {
	return hsqs_buffer_size(mapping->data.cl.buffer);
}

struct HsqsMemoryMapperImpl hsqs_mapper_impl_curl = {
		.init = hsqs_mapper_curl_init,
		.mapping = hsqs_mapper_curl_map,
		.size = hsqs_mapper_curl_size,
		.cleanup = hsqs_mapper_curl_cleanup,
		.map_data = hsqs_mapping_curl_data,
		.map_resize = hsqs_mapping_curl_resize,
		.map_size = hsqs_mapping_curl_size,
		.unmap = hsqs_mapping_curl_unmap,
};
