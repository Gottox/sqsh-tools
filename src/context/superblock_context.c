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
 * @file         superblock_context.c
 */

#include "../../include/sqsh_context_private.h"

#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"

enum SqshInitialized {
	SQSH_INITIALIZED_ID_TABLE = 1 << 0,
	SQSH_INITIALIZED_EXPORT_TABLE = 1 << 2,
	SQSH_INITIALIZED_XATTR_TABLE = 1 << 3,
	SQSH_INITIALIZED_FRAGMENT_TABLE = 1 << 4,
};

static uint16_t
log2_u32(uint32_t x) {
	if (x == 0) {
		return UINT16_MAX;
	} else {
		return sizeof(uint32_t) * 8 - 1 - __builtin_clz(x);
	}
}

static const struct SqshDataSuperblock *
get_header(const struct SqshSuperblockContext *context) {
	return (const struct SqshDataSuperblock *)sqsh__map_cursor_data(
			&context->cursor);
}

static bool
check_flag(
		const struct SqshSuperblockContext *context,
		enum SqshSuperblockFlags flag) {
	return sqsh_data_superblock_flags(get_header(context)) & flag;
}

int
sqsh__superblock_init(
		struct SqshSuperblockContext *context,
		struct SqshMapManager *map_manager) {
	int rv = 0;

	if (sqsh__map_manager_size(map_manager) < SQSH_SIZEOF_SUPERBLOCK) {
		rv = -SQSH_ERROR_SUPERBLOCK_TOO_SMALL;
		goto out;
	}

	rv = sqsh__map_cursor_init(
			&context->cursor, map_manager, 0, SQSH_SIZEOF_SUPERBLOCK);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_cursor_all(&context->cursor);
	if (rv < 0) {
		goto out;
	}
	const struct SqshDataSuperblock *superblock = get_header(context);

	if (sqsh_data_superblock_magic(superblock) != SQSH_SUPERBLOCK_MAGIC) {
		rv = -SQSH_ERROR_WRONG_MAGIC;
		goto out;
	}

	uint32_t block_size = sqsh_data_superblock_block_size(superblock);
	if (block_size < 4096 || block_size > 1048576) {
		rv = -SQSH_ERROR_BLOCKSIZE_MISSMATCH;
		goto out;
	}

	if (sqsh_data_superblock_block_log(superblock) != log2_u32(block_size)) {
		rv = -SQSH_ERROR_BLOCKSIZE_MISSMATCH;
		goto out;
	}

	if (sqsh_data_superblock_bytes_used(superblock) >
		sqsh__map_manager_size(map_manager)) {
		rv = -SQSH_ERROR_SIZE_MISSMATCH;
		goto out;
	}

out:
	return rv;
}

enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_compression_id(get_header(context));
}

uint64_t
sqsh_superblock_directory_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_directory_table_start(get_header(context));
}

uint64_t
sqsh_superblock_fragment_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_fragment_table_start(get_header(context));
}

uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_inode_count(get_header(context));
}

uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_inode_table_start(get_header(context));
}

uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_id_table_start(get_header(context));
}

uint16_t
sqsh_superblock_id_count(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_id_count(get_header(context));
}

uint64_t
sqsh_superblock_export_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_export_table_start(get_header(context));
}

uint64_t
sqsh_superblock_xattr_id_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_xattr_id_table_start(get_header(context));
}

uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_root_inode_ref(get_header(context));
}

bool
sqsh_superblock_has_compression_options(
		const struct SqshSuperblockContext *context) {
	return check_flag(context, SQSH_SUPERBLOCK_COMPRESSOR_OPTIONS);
}

bool
sqsh_superblock_has_fragments(const struct SqshSuperblockContext *context) {
	return !check_flag(context, SQSH_SUPERBLOCK_NO_FRAGMENTS);
}

bool
sqsh_superblock_has_export_table(const struct SqshSuperblockContext *context) {
	return check_flag(context, SQSH_SUPERBLOCK_EXPORTABLE);
}

uint32_t
sqsh_superblock_block_size(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_block_size(get_header(context));
}

uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_modification_time(get_header(context));
}

uint32_t
sqsh_superblock_fragment_entry_count(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_fragment_entry_count(get_header(context));
}

uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_bytes_used(get_header(context));
}

int
sqsh__superblock_cleanup(struct SqshSuperblockContext *superblock) {
	sqsh__map_cursor_cleanup(&superblock->cursor);
	return 0;
}
