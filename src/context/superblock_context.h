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
 * @file         superblock_context.h
 */

#ifndef SUPERBLOCK_CONTEXT_H

#define SUPERBLOCK_CONTEXT_H

#include "../data/superblock.h"
#include "../mapper/mapper.h"
#include "../utils.h"
#include <stdbool.h>
#include <stdint.h>

struct SqshSuperblock;

struct SqshSuperblockContext {
	const struct SqshSuperblock *superblock;
	struct SqshMapping mapping;
};

HSQS_NO_UNUSED int sqsh_superblock_init(
		struct SqshSuperblockContext *context, struct SqshMapper *mapper);

const void *sqsh_superblock_data_from_offset(
		const struct SqshSuperblockContext *context, uint64_t offset);

enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_directory_table_start(
		const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_fragment_table_start(
		const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblockContext *context);

uint16_t sqsh_superblock_id_count(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_export_table_start(const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_xattr_id_table_start(
		const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblockContext *context);

bool sqsh_superblock_has_fragments(const struct SqshSuperblockContext *context);

bool
sqsh_superblock_has_export_table(const struct SqshSuperblockContext *context);

bool sqsh_superblock_has_compression_options(
		const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_block_size(const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblockContext *context);

uint32_t sqsh_superblock_fragment_entry_count(
		const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblockContext *context);

int sqsh_superblock_cleanup(struct SqshSuperblockContext *superblock);

#endif /* end of include guard SUPERBLOCK_CONTEXT_H */
