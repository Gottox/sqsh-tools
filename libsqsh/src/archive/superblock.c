/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         superblock.c
 */

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>

#include <sqsh_error.h>

#include <sqsh_data_private.h>

enum SqshInitialized {
	SQSH_INITIALIZED_ID_TABLE = 1 << 0,
	SQSH_INITIALIZED_EXPORT_TABLE = 1 << 2,
	SQSH_INITIALIZED_XATTR_TABLE = 1 << 3,
	SQSH_INITIALIZED_FRAGMENT_TABLE = 1 << 4,
};

static const struct SqshDataSuperblock *
get_header(const struct SqshSuperblock *superblock) {
	return (const struct SqshDataSuperblock *)sqsh__map_reader_data(
			&superblock->cursor);
}

static bool
check_flag(
		const struct SqshSuperblock *superblock,
		enum SqshSuperblockFlags flag) {
	return sqsh__data_superblock_flags(get_header(superblock)) & flag;
}

int
sqsh__superblock_init(
		struct SqshSuperblock *superblock, struct SqshMapManager *map_manager) {
	int rv = 0;
	memset(superblock, 0, sizeof(*superblock));

	if (sqsh__map_manager_size(map_manager) <
		sizeof(struct SqshDataSuperblock)) {
		rv = -SQSH_ERROR_SUPERBLOCK_TOO_SMALL;
		goto out;
	}

	rv = sqsh__map_reader_init(
			&superblock->cursor, map_manager, 0,
			sizeof(struct SqshDataSuperblock));
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_reader_all(&superblock->cursor);
	if (rv < 0) {
		goto out;
	}
	const struct SqshDataSuperblock *header = get_header(superblock);

	if (sqsh__data_superblock_magic(header) != SQSH_SUPERBLOCK_MAGIC) {
		rv = -SQSH_ERROR_WRONG_MAGIC;
		goto out;
	}

	uint32_t block_size = sqsh__data_superblock_block_size(header);
	if (block_size < 4096 || block_size > 1048576) {
		rv = -SQSH_ERROR_BLOCKSIZE_MISMATCH;
		goto out;
	}

	if (sqsh__data_superblock_block_log(header) != sqsh__log2_u32(block_size)) {
		rv = -SQSH_ERROR_BLOCKSIZE_MISMATCH;
		goto out;
	}

	if (sqsh__data_superblock_bytes_used(header) >
		sqsh__map_manager_size(map_manager)) {
		rv = -SQSH_ERROR_SIZE_MISMATCH;
		goto out;
	}

out:
	return rv;
}

enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_compression_id(get_header(superblock));
}

uint64_t
sqsh_superblock_directory_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_directory_table_start(get_header(superblock));
}

uint64_t
sqsh_superblock_fragment_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_fragment_table_start(get_header(superblock));
}

uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_inode_count(get_header(superblock));
}

uint16_t
sqsh_superblock_version_major(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_version_major(get_header(superblock));
}

uint16_t
sqsh_superblock_version_minor(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_version_minor(get_header(superblock));
}

uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_inode_table_start(get_header(superblock));
}

uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_id_table_start(get_header(superblock));
}

uint16_t
sqsh_superblock_id_count(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_id_count(get_header(superblock));
}

uint64_t
sqsh_superblock_export_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_export_table_start(get_header(superblock));
}

uint64_t
sqsh_superblock_xattr_id_table_start(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_xattr_id_table_start(get_header(superblock));
}

uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_root_inode_ref(get_header(superblock));
}

bool
sqsh_superblock_has_compression_options(
		const struct SqshSuperblock *superblock) {
	return check_flag(superblock, SQSH_SUPERBLOCK_COMPRESSOR_OPTIONS);
}

bool
sqsh_superblock_has_fragments(const struct SqshSuperblock *superblock) {
	return !check_flag(superblock, SQSH_SUPERBLOCK_NO_FRAGMENTS);
}

bool
sqsh_superblock_has_export_table(const struct SqshSuperblock *superblock) {
	return check_flag(superblock, SQSH_SUPERBLOCK_EXPORTABLE);
}

bool
sqsh_superblock_has_xattr_table(const struct SqshSuperblock *superblock) {
	return !check_flag(superblock, SQSH_SUPERBLOCK_NO_XATTRS);
}

uint32_t
sqsh_superblock_block_size(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_block_size(get_header(superblock));
}

uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_modification_time(get_header(superblock));
}

uint32_t
sqsh_superblock_fragment_entry_count(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_fragment_entry_count(get_header(superblock));
}

uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblock *superblock) {
	return sqsh__data_superblock_bytes_used(get_header(superblock));
}

int
sqsh__superblock_cleanup(struct SqshSuperblock *superblock) {
	sqsh__map_reader_cleanup(&superblock->cursor);
	return 0;
}
