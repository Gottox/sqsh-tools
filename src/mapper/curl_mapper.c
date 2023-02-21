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

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

#include <curl/curl.h>
#include <inttypes.h>

#define SUPERBLOCK_REQUEST_SIZE \
	(SQSH_SIZEOF_SUPERBLOCK + SQSH_SIZEOF_COMPRESSION_OPTIONS)
#define CONTENT_RANGE "Content-Range: "
#define CONTENT_RANGE_LENGTH (sizeof(CONTENT_RANGE) - 1)

struct SqshCurlWriteInfo {
	uint8_t *buffer;
	size_t offset;
	size_t size;
	int rv;
};

static size_t
write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	size_t byte_size;
	size_t next_offset;
	struct SqshCurlWriteInfo *info = userdata;

	if (SQSH_MULT_OVERFLOW(size, nmemb, &byte_size)) {
		info->rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		return 0;
	}
	if (SQSH_ADD_OVERFLOW(info->offset, byte_size, &next_offset)) {
		info->rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		return 0;
	}

	if (next_offset > info->size) {
		info->rv = -SQSH_ERROR_MAPPER_MAP;
		return 0;
	}

	memcpy(&info->buffer[info->offset], ptr, byte_size);

	info->offset = next_offset;

	return byte_size;
}

static int
get_total_size(CURL *handle, uint64_t *total) {
	int rv = 0;
	uint64_t dummy;
	struct curl_header *header = NULL;
	static const char *format = "bytes %" PRIu64 "-%" PRIu64 "/%" PRIu64;
	int scanned_fields;

	rv = curl_easy_header(
			handle, "Content-Range", 0, CURLH_HEADER, -1, &header);
	if (rv != CURLE_OK) {
		return -SQSH_ERROR_CURL_INVALID_RANGE_HEADER;
	}

	scanned_fields = sscanf(header->value, format, &dummy, &dummy, total);

	if (scanned_fields != 3) {
		return -SQSH_ERROR_CURL_INVALID_RANGE_HEADER;
	}

	return 0;
}

static int
get_file_time(CURL *handle, uint64_t *file_time) {
	curl_off_t file_time_t;
	int rv;

	rv = curl_easy_getinfo(handle, CURLINFO_FILETIME_T, &file_time_t);
	// According to curl docs, file_time_t is set to -1 if the server does not
	// report a file time. We treat this as an error as we need to detect if
	// the file has changed.
	if (rv != CURLE_OK || file_time_t < 0) {
		return -SQSH_ERROR_MAPPER_MAP;
	}
	// We checked for negative values above, so this cast should be safe.
	*file_time = (uint64_t)file_time_t;

	return 0;
}

static CURL *
configure_handle(struct SqshMapper *mapper) {
	CURL *handle = mapper->data.cl.handle;
	curl_easy_reset(handle);
	curl_easy_setopt(handle, CURLOPT_URL, mapper->data.cl.url);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(handle, CURLOPT_FILETIME, 1L);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
	return handle;
}

static int
curl_download(
		struct SqshMapper *mapper, sqsh_index_t offset, size_t size,
		uint8_t **data) {
	// TODO: Declutter the initialisation of this function. Also check for
	// integer overflows.
	const uint64_t expected_size = mapper->data.cl.expected_size;
	const uint64_t expected_time = mapper->data.cl.expected_time;
	*data = calloc(size, sizeof(uint8_t));
	if (*data == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}

	struct SqshCurlWriteInfo write_info = {
			.size = size,
			.buffer = *data,
	};

	int rv = 0;
	// The actual max-size this string should ever use is 42, but we
	// add some padding to be a nice number. Not that 42 isn't nice.
	char range_buffer[64] = {0};
	CURL *handle = NULL;
	const uint64_t end_offset = offset + size - 1;
	uint64_t total_size = UINT64_MAX;
	uint64_t file_time = UINT64_MAX;
	long http_code = 0;

	rv = snprintf(
			range_buffer, sizeof(range_buffer), "%" PRIu64 "-%" PRIu64, offset,
			end_offset);
	if (rv >= (int)sizeof(range_buffer)) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}
	handle = configure_handle(mapper);
	curl_easy_setopt(handle, CURLOPT_RANGE, range_buffer);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &write_info);

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

	rv = get_total_size(handle, &total_size);
	if (rv < 0) {
		goto out;
	}

	rv = get_file_time(handle, &file_time);
	if (rv < 0) {
		goto out;
	}

	// UINT64_MAX is used to indicate, that expected_time or expected_size
	// has not been set yet. If the server reports UINT64_MAX, something fishy
	// is going on, so we bail out here.
	if (total_size == UINT64_MAX || file_time == UINT64_MAX) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_time == UINT64_MAX) {
		mapper->data.cl.expected_time = file_time;
	} else if (file_time != expected_time) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}

	if (expected_size == UINT64_MAX) {
		mapper->data.cl.expected_size = total_size;
	} else if (total_size != expected_size) {
		rv = -SQSH_ERROR_MAPPER_MAP;
		goto out;
	}
out:
	return rv;
}

static int
sqsh_mapper_curl_init(
		struct SqshMapper *mapper, const void *input, size_t size) {
	(void)size;
	int rv = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	mapper->data.cl.url = input;
	mapper->data.cl.handle = curl_easy_init();
	mapper->data.cl.expected_size = UINT64_MAX;
	mapper->data.cl.expected_time = UINT64_MAX;

	rv = pthread_mutex_init(&mapper->data.cl.lock, NULL);
	if (rv != 0) {
		rv = -SQSH_ERROR_MAPPER_INIT;
		goto out;
	}

	size_t block_size = sqsh__mapper_block_size(mapper);
	rv = curl_download(mapper, 0, block_size, &mapper->data.cl.header_cache);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

static int
sqsh_mapper_curl_map(
		struct SqshMapping *mapping, sqsh_index_t offset, size_t size) {
	int rv = 0;

	pthread_mutex_lock(&mapping->mapper->data.cl.lock);
	if (offset == 0 && mapping->mapper->data.cl.header_cache != NULL) {
		mapping->data.cl.data = mapping->mapper->data.cl.header_cache;
	} else {
		rv = curl_download(
				mapping->mapper, offset, size, &mapping->data.cl.data);
		if (rv < 0) {
			goto out;
		}
	}
	mapping->data.cl.size = size;

out:
	if (rv < 0) {
		sqsh__mapping_unmap(mapping);
	}
	pthread_mutex_unlock(&mapping->mapper->data.cl.lock);
	return rv;
}
static size_t
sqsh_mapper_curl_size(const struct SqshMapper *mapper) {
	return mapper->data.cl.expected_size;
}

static int
sqsh_mapper_curl_cleanup(struct SqshMapper *mapper) {
	pthread_mutex_destroy(&mapper->data.cl.lock);
	curl_easy_cleanup(mapper->data.cl.handle);
	return 0;
}

static int
sqsh_mapping_curl_unmap(struct SqshMapping *mapping) {
	free(mapping->data.cl.data);
	return 0;
}

static const uint8_t *
sqsh_mapping_curl_data(const struct SqshMapping *mapping) {
	return mapping->data.cl.data;
}

static const struct SqshMemoryMapperImpl impl = {
		.block_size_hint = 4096,
		.init = sqsh_mapper_curl_init,
		.mapping = sqsh_mapper_curl_map,
		.size = sqsh_mapper_curl_size,
		.cleanup = sqsh_mapper_curl_cleanup,
		.map_data = sqsh_mapping_curl_data,
		.unmap = sqsh_mapping_curl_unmap,
};
const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_curl = &impl;
