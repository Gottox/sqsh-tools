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
 * @file         archive.c
 */

#define _DEFAULT_SOURCE

#include "../../include/sqsh_archive_private.h"

#include "../../include/sqsh_data_private.h"

#include "../utils/utils.h"

#include <stdlib.h>
#include <string.h>

static const uint64_t NO_SEGMENT = 0xFFFFFFFFFFFFFFFF;

enum InitializedBitmap {
	INITIALIZED_ID_TABLE = 1 << 0,
	INITIALIZED_EXPORT_TABLE = 1 << 1,
	INITIALIZED_XATTR_TABLE = 1 << 2,
	INITIALIZED_FRAGMENT_TABLE = 1 << 3,
	INITIALIZED_DATA_COMPRESSION_MANAGER = 1 << 4,
	INITIALIZED_METABLOCK_COMPRESSION_MANAGER = 1 << 5,
	INITIALIZED_INODE_MAP = 1 << 6,
};

static bool
is_initialized(const struct SqshArchive *archive, enum InitializedBitmap mask) {
	return archive->initialized & mask;
}

struct SqshArchive *
sqsh_archive_new(
		const void *source, const struct SqshConfig *config, int *err) {
	struct SqshArchive *archive = calloc(1, sizeof(struct SqshArchive));
	if (archive == NULL) {
		return NULL;
	}
	*err = sqsh__archive_init(archive, source, config);
	if (*err < 0) {
		free(archive);
		return NULL;
	}
	return archive;
}

int
sqsh__archive_init(
		struct SqshArchive *archive, const void *source,
		const struct SqshConfig *config) {
	int rv = 0;

	// Initialize struct to 0, so in an error case we have a clean state that
	// we can call sqsh_mapper_cleanup on.
	memset(&archive->map_manager, 0, sizeof(struct SqshMapManager));

	if (config != NULL) {
		memcpy(&archive->config, config, sizeof(struct SqshConfig));
	} else {
		memset(&archive->config, 0, sizeof(struct SqshConfig));
	}

	// RECURSIVE is needed because: inode_map may access the export_table
	// during initialization.
	rv = sqsh_mutex_init_recursive(&archive->lock);
	if (rv < 0) {
		goto out;
	}

	config = sqsh_archive_config(archive);

	rv = sqsh__map_manager_init(&archive->map_manager, source, config);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__superblock_init(
			&archive->superblock, sqsh_archive_map_manager(archive));
	if (rv < 0) {
		goto out;
	}

	archive->zero_block = calloc(
			sqsh_superblock_block_size(&archive->superblock), sizeof(uint8_t));
	if (archive->zero_block == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	enum SqshSuperblockCompressionId compression_id =
			sqsh_superblock_compression_id(&archive->superblock);
	uint32_t data_block_size = sqsh_superblock_block_size(&archive->superblock);

	rv = sqsh__extractor_init(
			&archive->metablock_compression, compression_id,
			SQSH_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__extractor_init(
			&archive->data_compression, compression_id, data_block_size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		sqsh__archive_cleanup(archive);
	}
	return rv;
}

const struct SqshConfig *
sqsh_archive_config(const struct SqshArchive *archive) {
	return &archive->config;
}

const struct SqshSuperblock *
sqsh_archive_superblock(const struct SqshArchive *archive) {
	return &archive->superblock;
}

static uint64_t
get_data_segment_size(const struct SqshSuperblock *superblock) {
	const uint64_t inode_table_start =
			sqsh_superblock_inode_table_start(superblock);
	uint64_t res;
	// BUG: This function does not return exact results. It may report values
	// that are too large, as it does not take into account the size of the
	// compression options. This is not a problem for the current implementation
	// as this size is only used for finding upper limits for the extract
	// manager.
	if (SQSH_SUB_OVERFLOW(inode_table_start, SQSH_SIZEOF_SUPERBLOCK, &res)) {
		return inode_table_start;
	}
	return res;
}

int
sqsh__archive_metablock_extract_manager(
		struct SqshArchive *archive,
		struct SqshExtractManager **metablock_extract_manager) {
	int rv = 0;

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!is_initialized(archive, INITIALIZED_METABLOCK_COMPRESSION_MANAGER)) {
		const struct SqshSuperblock *superblock =
				sqsh_archive_superblock(archive);
		uint64_t range;
		if (SQSH_SUB_OVERFLOW(
					sqsh_superblock_bytes_used(superblock),
					get_data_segment_size(superblock), &range)) {
			rv = -SQSH_ERROR_INTEGER_OVERFLOW;
			return rv;
		}
		const size_t capacity = SQSH_DIVIDE_CEIL(
				range, SQSH_SIZEOF_METABLOCK + SQSH_METABLOCK_BLOCK_SIZE);

		rv = sqsh__extract_manager_init(
				&archive->metablock_extract_manager, archive,
				&archive->metablock_compression, capacity);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_METABLOCK_COMPRESSION_MANAGER;
	}
	*metablock_extract_manager = &archive->metablock_extract_manager;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh__archive_data_extract_manager(
		struct SqshArchive *archive,
		struct SqshExtractManager **data_extract_manager) {
	int rv = 0;

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!is_initialized(archive, INITIALIZED_DATA_COMPRESSION_MANAGER)) {
		const struct SqshSuperblock *superblock =
				sqsh_archive_superblock(archive);
		const uint64_t range = get_data_segment_size(superblock);
		const size_t capacity =
				SQSH_DIVIDE_CEIL(range, sqsh_superblock_block_size(superblock));

		rv = sqsh__extract_manager_init(
				&archive->data_extract_manager, archive,
				&archive->data_compression, capacity);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_DATA_COMPRESSION_MANAGER;
	}
	*data_extract_manager = &archive->data_extract_manager;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh_archive_id_table(
		struct SqshArchive *archive, struct SqshIdTable **id_table) {
	int rv = 0;

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!is_initialized(archive, INITIALIZED_ID_TABLE)) {
		rv = sqsh__id_table_init(&archive->id_table, archive);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_ID_TABLE;
	}
	*id_table = &archive->id_table;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh_archive_export_table(
		struct SqshArchive *archive, struct SqshExportTable **export_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_export_table_start(&archive->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_EXPORT_TABLE;
	}

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!(archive->initialized & INITIALIZED_EXPORT_TABLE)) {
		rv = sqsh__export_table_init(&archive->export_table, archive);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_EXPORT_TABLE;
	}
	*export_table = &archive->export_table;

out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh_archive_fragment_table(
		struct SqshArchive *archive,
		struct SqshFragmentTable **fragment_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_fragment_table_start(&archive->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_FRAGMENT_TABLE;
	}

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!is_initialized(archive, INITIALIZED_FRAGMENT_TABLE)) {
		rv = sqsh__fragment_table_init(&archive->fragment_table, archive);

		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_FRAGMENT_TABLE;
	}
	*fragment_table = &archive->fragment_table;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh_archive_inode_map(
		struct SqshArchive *archive, struct SqshInodeMap **inode_map) {
	int rv = 0;

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!(archive->initialized & INITIALIZED_INODE_MAP)) {
		rv = sqsh__inode_map_init(&archive->inode_map, archive);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_INODE_MAP;
	}
	*inode_map = &archive->inode_map;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

int
sqsh_archive_xattr_table(
		struct SqshArchive *archive, struct SqshXattrTable **xattr_table) {
	int rv = 0;
	uint64_t table_start =
			sqsh_superblock_xattr_id_table_start(&archive->superblock);
	if (table_start == NO_SEGMENT) {
		return -SQSH_ERROR_NO_XATTR_TABLE;
	}

	rv = sqsh_mutex_lock(&archive->lock);
	if (rv < 0) {
		goto out;
	}
	if (!(archive->initialized & INITIALIZED_XATTR_TABLE)) {
		rv = sqsh__xattr_table_init(&archive->xattr_table, archive);
		if (rv < 0) {
			goto out;
		}
		archive->initialized |= INITIALIZED_XATTR_TABLE;
	}
	*xattr_table = &archive->xattr_table;
out:
	sqsh_mutex_unlock(&archive->lock);
	return rv;
}

const uint8_t *
sqsh__archive_zero_block(const struct SqshArchive *archive) {
	return archive->zero_block;
}

struct SqshMapManager *
sqsh_archive_map_manager(struct SqshArchive *archive) {
	return &archive->map_manager;
}

int
sqsh__archive_cleanup(struct SqshArchive *archive) {
	int rv = 0;

	if (is_initialized(archive, INITIALIZED_ID_TABLE)) {
		sqsh__id_table_cleanup(&archive->id_table);
	}
	if (is_initialized(archive, INITIALIZED_EXPORT_TABLE)) {
		sqsh__export_table_cleanup(&archive->export_table);
	}
	if (is_initialized(archive, INITIALIZED_XATTR_TABLE)) {
		sqsh__xattr_table_cleanup(&archive->xattr_table);
	}
	if (is_initialized(archive, INITIALIZED_FRAGMENT_TABLE)) {
		sqsh__fragment_table_cleanup(&archive->fragment_table);
	}
	if (is_initialized(archive, INITIALIZED_DATA_COMPRESSION_MANAGER)) {
		sqsh__extract_manager_cleanup(&archive->data_extract_manager);
	}
	if (is_initialized(archive, INITIALIZED_METABLOCK_COMPRESSION_MANAGER)) {
		sqsh__extract_manager_cleanup(&archive->metablock_extract_manager);
	}
	if (is_initialized(archive, INITIALIZED_INODE_MAP)) {
		sqsh__inode_map_cleanup(&archive->inode_map);
	}
	sqsh__extractor_cleanup(&archive->data_compression);
	sqsh__extractor_cleanup(&archive->metablock_compression);
	sqsh__superblock_cleanup(&archive->superblock);
	sqsh__map_manager_cleanup(&archive->map_manager);

	free(archive->zero_block);

	sqsh_mutex_destroy(&archive->lock);

	return rv;
}

int
sqsh_archive_free(struct SqshArchive *archive) {
	if (archive == NULL) {
		return 0;
	}
	int rv = sqsh__archive_cleanup(archive);
	free(archive);
	return rv;
}
