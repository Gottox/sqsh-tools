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
 * @file        : superblock_context
 * @created     : Monday Oct 11, 2021 13:41:47 CEST
 */

#include "superblock_context.h"
#include "../data/superblock.h"
#include "../error.h"
#include <stdint.h>

static const uint32_t SUPERBLOCK_MAGIC = 0x73717368;

enum HsqsInitialized {
	HSQS_INITIALIZED_ID_TABLE = 1 << 0,
	HSQS_INITIALIZED_EXPORT_TABLE = 1 << 2,
	HSQS_INITIALIZED_XATTR_TABLE = 1 << 3,
	HSQS_INITIALIZED_FRAGMENT_TABLE = 1 << 4,
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
		const struct HsqsSuperblockContext *context,
		enum HsqsSuperblockFlags flag) {
	return hsqs_data_superblock_flags(context->superblock) & flag;
}

int
hsqs_superblock_init(
		struct HsqsSuperblockContext *context, struct HsqsMapper *mapper) {
	int rv = 0;

	if (hsqs_mapper_size(mapper) < HSQS_SIZEOF_SUPERBLOCK) {
		rv = -HSQS_ERROR_SUPERBLOCK_TOO_SMALL;
		goto out;
	}

	rv = hsqs_mapper_map(&context->map, mapper, 0, HSQS_SIZEOF_SUPERBLOCK);
	if (rv < 0) {
		goto out;
	}
	const struct HsqsSuperblock *superblock =
			(const struct HsqsSuperblock *)hsqs_map_data(&context->map);

	if (hsqs_data_superblock_magic(superblock) != SUPERBLOCK_MAGIC) {
		rv = -HSQS_ERROR_WRONG_MAGIC;
		goto out;
	}

	uint32_t block_size = hsqs_data_superblock_block_size(superblock);
	if (block_size < 4096 || block_size > 1048576) {
		rv = -HSQS_ERROR_BLOCKSIZE_MISSMATCH;
		goto out;
	}

	if (hsqs_data_superblock_block_log(superblock) != log2_u32(block_size)) {
		rv = -HSQS_ERROR_BLOCKSIZE_MISSMATCH;
		goto out;
	}

	if (hsqs_data_superblock_bytes_used(superblock) >
		hsqs_mapper_size(mapper)) {
		rv = -HSQS_ERROR_SIZE_MISSMATCH;
		goto out;
	}

	context->superblock = superblock;

out:
	return rv;
}

const void *
hsqs_superblock_data_from_offset(
		const struct HsqsSuperblockContext *context, uint64_t offset) {
	const uint8_t *tmp = (uint8_t *)context->superblock;
	if (offset > hsqs_superblock_bytes_used(context)) {
		return NULL;
	}
	if (offset < HSQS_SIZEOF_SUPERBLOCK) {
		return NULL;
	}

	return &tmp[offset];
}

enum HsqsSuperblockCompressionId
hsqs_superblock_compression_id(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_compression_id(context->superblock);
}

uint64_t
hsqs_superblock_directory_table_start(
		const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_directory_table_start(context->superblock);
}

uint64_t
hsqs_superblock_fragment_table_start(
		const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_fragment_table_start(context->superblock);
}

uint32_t
hsqs_superblock_inode_count(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_inode_count(context->superblock);
}

uint64_t
hsqs_superblock_inode_table_start(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_inode_table_start(context->superblock);
}

uint64_t
hsqs_superblock_id_table_start(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_id_table_start(context->superblock);
}

uint16_t
hsqs_superblock_id_count(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_id_count(context->superblock);
}

uint64_t
hsqs_superblock_export_table_start(
		const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_export_table_start(context->superblock);
}

uint64_t
hsqs_superblock_xattr_id_table_start(
		const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_xattr_id_table_start(context->superblock);
}

uint64_t
hsqs_superblock_inode_root_ref(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_root_inode_ref(context->superblock);
}

bool
hsqs_superblock_has_compression_options(
		const struct HsqsSuperblockContext *context) {
	return check_flag(context, HSQS_SUPERBLOCK_COMPRESSOR_OPTIONS);
}

bool
hsqs_superblock_has_fragments(const struct HsqsSuperblockContext *context) {
	return !check_flag(context, HSQS_SUPERBLOCK_NO_FRAGMENTS);
}

bool
hsqs_superblock_has_export_table(const struct HsqsSuperblockContext *context) {
	return check_flag(context, HSQS_SUPERBLOCK_EXPORTABLE);
}

uint32_t
hsqs_superblock_block_size(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_block_size(context->superblock);
}

uint32_t
hsqs_superblock_fragment_entry_count(
		const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_fragment_entry_count(context->superblock);
}

uint64_t
hsqs_superblock_bytes_used(const struct HsqsSuperblockContext *context) {
	return hsqs_data_superblock_bytes_used(context->superblock);
}

int
hsqs_superblock_cleanup(struct HsqsSuperblockContext *superblock) {
	hsqs_map_unmap(&superblock->map);
	return 0;
}
