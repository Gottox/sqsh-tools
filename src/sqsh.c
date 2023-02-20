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
 * @file         sqsh.c
 */

#include "../include/sqsh_private.h"

#include "../include/sqsh_data.h"

#include <stdlib.h>
#include <string.h>

static const uint64_t NO_SEGMENT = 0xFFFFFFFFFFFFFFFF;

enum InitializedBitmap {
	INITIALIZED_ID_TABLE = 1 << 0,
	INITIALIZED_EXPORT_TABLE = 1 << 1,
	INITIALIZED_XATTR_TABLE = 1 << 2,
	INITIALIZED_FRAGMENT_TABLE = 1 << 3,
};

static bool
is_initialized(const struct Sqsh *sqsh, enum InitializedBitmap mask) {
	return sqsh->initialized & mask;
}

struct Sqsh *
sqsh_new(const void *source, const struct SqshConfig *config, int *err) {
	struct Sqsh *sqsh = calloc(1, sizeof(struct Sqsh));
	if (sqsh == NULL) {
		return NULL;
	}
	*err = sqsh__init(sqsh, source, config);
	if (*err < 0) {
		free(sqsh);
		return NULL;
	}
	return sqsh;
}

int
sqsh__init(
		struct Sqsh *sqsh, const void *source,
		const struct SqshConfig *config) {
	int rv = 0;

	// Initialize struct to 0, so in an error case we have a clean state that
	// we can call sqsh_mapper_cleanup on.
	memset(&sqsh->mapper, 0, sizeof(struct SqshMapper));

	if (config != NULL) {
		config = memcpy(&sqsh->config, config, sizeof(struct SqshConfig));
	} else {
		config = memset(&sqsh->config, 0, sizeof(struct SqshConfig));
	}

	config = sqsh_config(sqsh);

	rv = sqsh_mapper_init(&sqsh->mapper, source, config);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__superblock_init(&sqsh->superblock, &sqsh->mapper);
	if (rv < 0) {
		goto out;
	}

	enum SqshSuperblockCompressionId compression_id =
			sqsh_superblock_compression_id(&sqsh->superblock);
	uint32_t data_block_size = sqsh_superblock_block_size(&sqsh->superblock);

	rv = sqsh__compression_init(
			&sqsh->metablock_compression, compression_id,
			SQSH_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__compression_init(
			&sqsh->data_compression, compression_id, data_block_size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		sqsh__cleanup(sqsh);
	}
	return rv;
}

const struct SqshConfig *
sqsh_config(const struct Sqsh *sqsh) {
	return &sqsh->config;
}

const struct SqshSuperblockContext *
sqsh_superblock(const struct Sqsh *sqsh) {
	return &sqsh->superblock;
}

int
sqsh_id_table(struct Sqsh *sqsh, struct SqshTable **id_table) {
	int rv = 0;
	uint64_t table_start = sqsh_superblock_id_table_start(&sqsh->superblock);

	if (!is_initialized(sqsh, INITIALIZED_ID_TABLE)) {
		rv = sqsh__table_init(
				&sqsh->id_table, sqsh, table_start, sizeof(uint32_t),
				sqsh_superblock_id_count(&sqsh->superblock));
		if (rv < 0) {
			goto out;
		}
		sqsh->initialized |= INITIALIZED_ID_TABLE;
	}
	*id_table = &sqsh->id_table;
out:
	return rv;
}

int
sqsh_export_table(struct Sqsh *sqsh, struct SqshTable **export_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_export_table_start(&sqsh->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_EXPORT_TABLE;
	}

	if (!(sqsh->initialized & INITIALIZED_EXPORT_TABLE)) {
		rv = sqsh__table_init(
				&sqsh->export_table, sqsh, table_start, sizeof(uint64_t),
				sqsh_superblock_inode_count(&sqsh->superblock));
		if (rv < 0) {
			goto out;
		}
		sqsh->initialized |= INITIALIZED_EXPORT_TABLE;
	}
	*export_table = &sqsh->export_table;
out:
	return rv;
}

int
sqsh_fragment_table(
		struct Sqsh *sqsh, struct SqshFragmentTable **fragment_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_fragment_table_start(&sqsh->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_FRAGMENT_TABLE;
	}

	if (!is_initialized(sqsh, INITIALIZED_FRAGMENT_TABLE)) {
		rv = sqsh__fragment_table_init(&sqsh->fragment_table, sqsh);

		if (rv < 0) {
			goto out;
		}
		sqsh->initialized |= INITIALIZED_FRAGMENT_TABLE;
	}
	*fragment_table = &sqsh->fragment_table;
out:
	return rv;
}

int
sqsh_xattr_table(struct Sqsh *sqsh, struct SqshXattrTable **xattr_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_xattr_id_table_start(&sqsh->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_XATTR_TABLE;
	}

	if (!(sqsh->initialized & INITIALIZED_XATTR_TABLE)) {
		rv = sqsh__xattr_table_init(&sqsh->xattr_table, sqsh);
		if (rv < 0) {
			goto out;
		}
		sqsh->initialized |= INITIALIZED_XATTR_TABLE;
	}
	*xattr_table = &sqsh->xattr_table;
out:
	return rv;
}

const struct SqshCompression *
sqsh_compression_data(const struct Sqsh *sqsh) {
	return &sqsh->data_compression;
}

const struct SqshCompression *
sqsh_compression_metablock(const struct Sqsh *sqsh) {
	return &sqsh->metablock_compression;
}

struct SqshMapper *
sqsh_mapper(struct Sqsh *sqsh) {
	return &sqsh->mapper;
}

int
sqsh__cleanup(struct Sqsh *sqsh) {
	int rv = 0;

	if (is_initialized(sqsh, INITIALIZED_ID_TABLE)) {
		sqsh_table_cleanup(&sqsh->id_table);
	}
	if (is_initialized(sqsh, INITIALIZED_EXPORT_TABLE)) {
		sqsh_table_cleanup(&sqsh->export_table);
	}
	if (is_initialized(sqsh, INITIALIZED_XATTR_TABLE)) {
		sqsh__xattr_table_cleanup(&sqsh->xattr_table);
	}
	if (is_initialized(sqsh, INITIALIZED_FRAGMENT_TABLE)) {
		sqsh__fragment_table_cleanup(&sqsh->fragment_table);
	}
	sqsh__compression_cleanup(&sqsh->data_compression);
	sqsh__compression_cleanup(&sqsh->metablock_compression);
	sqsh__superblock_cleanup(&sqsh->superblock);
	sqsh_mapper_cleanup(&sqsh->mapper);

	return rv;
}

int
sqsh_free(struct Sqsh *sqsh) {
	if (sqsh == NULL) {
		return 0;
	}
	int rv = sqsh__cleanup(sqsh);
	free(sqsh);
	return rv;
}
