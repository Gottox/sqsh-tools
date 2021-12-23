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
 * @file        : hsqs
 * @created     : Friday Apr 30, 2021 11:09:40 CEST
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "compression/compression.h"
#include "hsqs.h"

static const uint64_t NO_SEGMENT = 0xFFFFFFFFFFFFFFFF;

enum InitializedBitmap {
	INITIALIZED_ID_TABLE = 1 << 0,
	INITIALIZED_EXPORT_TABLE = 1 << 2,
	INITIALIZED_XATTR_TABLE = 1 << 3,
	INITIALIZED_FRAGMENT_TABLE = 1 << 4,
	INITIALIZED_COMPRESSION_OPTIONS = 1 << 5,
	INITIALIZED_TRAILING_BYTES = 1 << 6,
};

static bool
is_initialized(const struct Hsqs *hsqs, enum InitializedBitmap mask) {
	return hsqs->initialized & mask;
}

static int
init(struct Hsqs *hsqs) {
	int rv = 0;

	rv = hsqs_superblock_init(&hsqs->superblock, &hsqs->mapper);
	if (rv < 0) {
		goto out;
	}

	if (hsqs_superblock_has_compression_options(&hsqs->superblock)) {
		hsqs->initialized |= INITIALIZED_COMPRESSION_OPTIONS;
		rv = hsqs_compression_options_init(&hsqs->compression_options, hsqs);
		if (rv < 0) {
			goto out;
		}
	}

out:
	if (rv < 0) {
		hsqs_cleanup(hsqs);
	}
	return rv;
}

int
hsqs_init(struct Hsqs *hsqs, const uint8_t *buffer, const size_t size) {
	int rv = 0;

	rv = hsqs_mapper_init_static(&hsqs->mapper, buffer, size);
	if (rv < 0) {
		return rv;
	}

	return init(hsqs);
}

int
hsqs_open(struct Hsqs *hsqs, const char *path) {
	int rv = 0;

	rv = hsqs_mapper_init_mmap(&hsqs->mapper, path);
	if (rv < 0) {
		return rv;
	}

	return init(hsqs);
}

int
hsqs_id_table(struct Hsqs *hsqs, struct HsqsTable **id_table) {
	int rv = 0;
	uint64_t table_start = hsqs_superblock_id_table_start(&hsqs->superblock);
	if (table_start == NO_SEGMENT) {
		return -HSQS_ERROR_NO_XATTR_TABLE;
	}

	if (!is_initialized(hsqs, INITIALIZED_ID_TABLE)) {
		rv = hsqs_table_init(
				&hsqs->id_table, hsqs, table_start, sizeof(uint32_t),
				hsqs_superblock_id_count(&hsqs->superblock));
		if (rv < 0) {
			goto out;
		}
		hsqs->initialized |= INITIALIZED_ID_TABLE;
	}
	*id_table = &hsqs->id_table;
out:
	return rv;
}

int
hsqs_export_table(struct Hsqs *hsqs, struct HsqsTable **export_table) {
	int rv = 0;
	uint64_t table_start =
			hsqs_superblock_export_table_start(&hsqs->superblock);
	if (table_start == NO_SEGMENT) {
		return -HSQS_ERROR_NO_XATTR_TABLE;
	}

	if (!(hsqs->initialized & INITIALIZED_EXPORT_TABLE)) {
		rv = hsqs_table_init(
				&hsqs->export_table, hsqs, table_start, sizeof(uint64_t),
				hsqs_superblock_inode_count(&hsqs->superblock));
		if (rv < 0) {
			goto out;
		}
		hsqs->initialized |= INITIALIZED_EXPORT_TABLE;
	}
	*export_table = &hsqs->export_table;
out:
	return rv;
}

int
hsqs_fragment_table(
		struct Hsqs *hsqs, struct HsqsFragmentTable **fragment_table) {
	int rv = 0;
	uint64_t table_start =
			hsqs_superblock_fragment_table_start(&hsqs->superblock);
	if (table_start == NO_SEGMENT) {
		return -HSQS_ERROR_NO_XATTR_TABLE;
	}

	if (!is_initialized(hsqs, INITIALIZED_FRAGMENT_TABLE)) {
		rv = hsqs_fragment_table_init(&hsqs->fragment_table, hsqs);

		if (rv < 0) {
			goto out;
		}
		hsqs->initialized |= INITIALIZED_FRAGMENT_TABLE;
	}
	*fragment_table = &hsqs->fragment_table;
out:
	return rv;
}

int
hsqs_xattr_table(struct Hsqs *hsqs, struct HsqsXattrTable **xattr_table) {
	int rv = 0;
	uint64_t table_start =
			hsqs_superblock_xattr_id_table_start(&hsqs->superblock);

	if (table_start == NO_SEGMENT) {
		return -HSQS_ERROR_NO_XATTR_TABLE;
	}

	if (!(hsqs->initialized & INITIALIZED_XATTR_TABLE)) {
		rv = hsqs_xattr_table_init(&hsqs->xattr_table, hsqs);
		if (rv < 0) {
			goto out;
		}
		hsqs->initialized |= INITIALIZED_XATTR_TABLE;
	}
	*xattr_table = &hsqs->xattr_table;
out:
	return rv;
}

int
hsqs_compression_options(
		struct Hsqs *hsqs,
		struct HsqsCompressionOptionsContext **compression_options) {
	if (hsqs_superblock_has_compression_options(&hsqs->superblock)) {
		*compression_options = &hsqs->compression_options;
		return 0;
	} else {
		return -HSQS_ERROR_NO_COMPRESSION_OPTIONS;
	}
}

struct HsqsSuperblockContext *
hsqs_superblock(struct Hsqs *hsqs) {
	return &hsqs->superblock;
}

struct HsqsMapper *
hsqs_mapper(struct Hsqs *hsqs) {
	return &hsqs->mapper;
}

const uint8_t *
hsqs_trailing_bytes(struct Hsqs *hsqs) {
	if (!is_initialized(hsqs, INITIALIZED_TRAILING_BYTES)) {
		struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
		struct HsqsMapper *mapper = hsqs_mapper(hsqs);
		uint64_t trailing_start = hsqs_superblock_bytes_used(superblock);
		size_t archive_size = hsqs_mapper_size(mapper);
		uint64_t trailing_size;

		if (archive_size <= trailing_start) {
			return NULL;
		}

		trailing_size = archive_size - trailing_start;

		hsqs_mapper_map(
				&hsqs->trailing_map, mapper, trailing_start, trailing_size);
	}
	return hsqs_map_data(&hsqs->trailing_map);
}
size_t
hsqs_trailing_bytes_size(struct Hsqs *hsqs) {
	if (!is_initialized(hsqs, INITIALIZED_TRAILING_BYTES)) {
		if (hsqs_map_data(&hsqs->trailing_map) == NULL) {
			return 0;
		}
	}
	return hsqs_map_size(&hsqs->trailing_map);
}

int
hsqs_cleanup(struct Hsqs *hsqs) {
	int rv = 0;

	if (is_initialized(hsqs, INITIALIZED_ID_TABLE)) {
		hsqs_table_cleanup(&hsqs->id_table);
	}
	if (is_initialized(hsqs, INITIALIZED_EXPORT_TABLE)) {
		hsqs_table_cleanup(&hsqs->export_table);
	}
	if (is_initialized(hsqs, INITIALIZED_XATTR_TABLE)) {
		hsqs_xattr_table_cleanup(&hsqs->xattr_table);
	}
	if (is_initialized(hsqs, INITIALIZED_FRAGMENT_TABLE)) {
		hsqs_fragment_table_cleanup(&hsqs->fragment_table);
	}
	if (is_initialized(hsqs, INITIALIZED_COMPRESSION_OPTIONS)) {
		hsqs_compression_options_cleanup(&hsqs->compression_options);
	}
	hsqs_superblock_cleanup(&hsqs->superblock);
	hsqs_mapper_cleanup(&hsqs->mapper);

	return rv;
}
