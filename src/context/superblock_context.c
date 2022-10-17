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
 * @author       Enno Boland (mail@eboland.de)
 * @file         superblock_context.c
 */

#include "superblock_context.h"
#include "../data/superblock_data.h"
#include "../error.h"
#include <stdint.h>

static const uint32_t SUPERBLOCK_MAGIC = 0x73717368;

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

static bool
check_flag(
		const struct SqshSuperblockContext *context,
		enum SqshSuperblockFlags flag) {
	return sqsh_data_superblock_flags(context->superblock) & flag;
}

int
sqsh_superblock_init(
		struct SqshSuperblockContext *context, struct SqshMapper *mapper) {
	int rv = 0;

	if (sqsh_mapper_size(mapper) < SQSH_SIZEOF_SUPERBLOCK) {
		rv = -SQSH_ERROR_SUPERBLOCK_TOO_SMALL;
		goto out;
	}

	rv = sqsh_mapper_map(&context->mapping, mapper, 0, SQSH_SIZEOF_SUPERBLOCK);
	if (rv < 0) {
		goto out;
	}
	const struct SqshSuperblock *superblock =
			(const struct SqshSuperblock *)sqsh_mapping_data(&context->mapping);

	if (sqsh_data_superblock_magic(superblock) != SUPERBLOCK_MAGIC) {
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
		sqsh_mapper_size(mapper)) {
		rv = -SQSH_ERROR_SIZE_MISSMATCH;
		goto out;
	}

	context->superblock = superblock;

out:
	return rv;
}

const void *
sqsh_superblock_data_from_offset(
		const struct SqshSuperblockContext *context, uint64_t offset) {
	const uint8_t *tmp = (uint8_t *)context->superblock;
	if (offset > sqsh_superblock_bytes_used(context)) {
		return NULL;
	}
	if (offset < SQSH_SIZEOF_SUPERBLOCK) {
		return NULL;
	}

	return &tmp[offset];
}

enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_compression_id(context->superblock);
}

uint64_t
sqsh_superblock_directory_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_directory_table_start(context->superblock);
}

uint64_t
sqsh_superblock_fragment_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_fragment_table_start(context->superblock);
}

uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_inode_count(context->superblock);
}

uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_inode_table_start(context->superblock);
}

uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_id_table_start(context->superblock);
}

uint16_t
sqsh_superblock_id_count(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_id_count(context->superblock);
}

uint64_t
sqsh_superblock_export_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_export_table_start(context->superblock);
}

uint64_t
sqsh_superblock_xattr_id_table_start(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_xattr_id_table_start(context->superblock);
}

uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_root_inode_ref(context->superblock);
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
	return sqsh_data_superblock_block_size(context->superblock);
}

uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_modification_time(context->superblock);
}

uint32_t
sqsh_superblock_fragment_entry_count(
		const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_fragment_entry_count(context->superblock);
}

uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblockContext *context) {
	return sqsh_data_superblock_bytes_used(context->superblock);
}

int
sqsh_superblock_cleanup(struct SqshSuperblockContext *superblock) {
	sqsh_mapping_unmap(&superblock->mapping);
	return 0;
}
