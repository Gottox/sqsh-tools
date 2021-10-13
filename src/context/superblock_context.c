/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2018, Enno Boland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock_context
 * @created     : Monday Oct 11, 2021 13:41:47 CEST
 */

#include "superblock_context.h"
#include "../data/superblock.h"
#include "../error.h"
#include <stdint.h>

const static uint32_t SUPERBLOCK_MAGIC = 0x73717368;

int
squash_superblock_init(struct SquashSuperblockContext *context,
		const uint8_t *buffer, size_t size) {
	const struct SquashSuperblock *superblock =
			(const struct SquashSuperblock *)buffer;
	if (size < SQUASH_SIZEOF_SUPERBLOCK) {
		return -SQUASH_ERROR_SUPERBLOCK_TOO_SMALL;
	}

	// Do not use the getter here as it may change the endianess. We don't want
	// that here.
	if (squash_data_superblock_magic(superblock) != SUPERBLOCK_MAGIC) {
		return -SQUASH_ERROR_WRONG_MAGIG;
	}

	if (squash_data_superblock_block_log(superblock) !=
			squash_log2_u32(squash_data_superblock_block_size(superblock))) {
		return -SQUASH_ERROR_BLOCKSIZE_MISSMATCH;
	}

	if (squash_data_superblock_bytes_used(superblock) > size) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	context->superblock = superblock;

	return 0;
}

const void *
squash_superblock_data_from_offset(
		const struct SquashSuperblockContext *context, uint64_t offset) {
	const uint8_t *tmp = (uint8_t *)context->superblock;
	if (offset > squash_superblock_bytes_used(context)) {
		return NULL;
	}
	if (offset < SQUASH_SIZEOF_SUPERBLOCK) {
		return NULL;
	}

	return &tmp[offset];
}

uint64_t
squash_superblock_directory_table_start(
		const struct SquashSuperblockContext *context) {
	return squash_data_superblock_directory_table_start(context->superblock);
}

uint64_t
squash_superblock_fragment_table_start(
		const struct SquashSuperblockContext *context) {
	return squash_data_superblock_fragment_table_start(context->superblock);
}

uint64_t
squash_superblock_inode_table_start(
		const struct SquashSuperblockContext *context) {
	return squash_data_superblock_inode_table_start(context->superblock);
}

uint64_t
squash_superblock_inode_root_ref(
		const struct SquashSuperblockContext *context) {
	return squash_data_superblock_root_inode_ref(context->superblock);
}

bool
squash_superblock_has_fragments(const struct SquashSuperblockContext *context) {
	return !(squash_data_superblock_flags(context->superblock) &
			SQUASH_SUPERBLOCK_NO_FRAGMENTS);
}

uint32_t
squash_superblock_block_size(const struct SquashSuperblockContext *context) {
	return squash_data_superblock_block_size(context->superblock);
}

uint32_t
squash_superblock_fragment_entry_count(
		const struct SquashSuperblockContext *context) {
	return squash_data_superblock_fragment_entry_count(context->superblock);
}

uint64_t
squash_superblock_bytes_used(const struct SquashSuperblockContext *context) {
	return squash_data_superblock_bytes_used(context->superblock);
}

int
squash_superblock_cleanup(struct SquashSuperblockContext *superblock) {
	return 0;
}
