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
 * @created     : Monday Oct 11, 2021 13:41:59 CEST
 */

#include "../utils.h"
#include "table_context.h"
#include "xattr_table_context.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef SUPERBLOCK_CONTEXT_H

#define SUPERBLOCK_CONTEXT_H

struct SquashSuperblock;

struct SquashSuperblockContext {
	const struct SquashSuperblock *superblock;
	struct SquashTableContext id_table;
	struct SquashXattrTableContext xattr_table;
};

SQUASH_NO_UNUSED int squash_superblock_init(
		struct SquashSuperblockContext *context, const uint8_t *buffer,
		size_t size);

const void *squash_superblock_data_from_offset(
		const struct SquashSuperblockContext *context, uint64_t offset);

uint64_t squash_superblock_directory_table_start(
		const struct SquashSuperblockContext *context);

uint64_t squash_superblock_fragment_table_start(
		const struct SquashSuperblockContext *context);

uint64_t squash_superblock_inode_table_start(
		const struct SquashSuperblockContext *context);

uint64_t
squash_superblock_inode_root_ref(const struct SquashSuperblockContext *context);

bool
squash_superblock_has_fragments(const struct SquashSuperblockContext *context);

uint32_t
squash_superblock_block_size(const struct SquashSuperblockContext *context);

uint32_t squash_superblock_fragment_entry_count(
		const struct SquashSuperblockContext *context);

uint64_t
squash_superblock_bytes_used(const struct SquashSuperblockContext *context);

struct SquashTableContext *
squash_superblock_id_table(struct SquashSuperblockContext *context);

int squash_superblock_cleanup(struct SquashSuperblockContext *superblock);

#endif /* end of include guard SUPERBLOCK_CONTEXT_H */
