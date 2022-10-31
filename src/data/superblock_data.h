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
 * @file         superblock_data.h
 */

#include "../utils.h"
#include <stddef.h>
#include <stdint.h>

#ifndef SQSH_SUPERBLOCK_DATA_H

#define SQSH_SUPERBLOCK_DATA_H

#define SQSH_SIZEOF_SUPERBLOCK 96

struct SQSH_UNALIGNED SqshSuperblock;

int
sqsh_data_superblock_init(const struct SqshSuperblock *superblock, size_t size);

uint32_t sqsh_data_superblock_magic(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_inode_count(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_modification_time(const struct SqshSuperblock *superblock);
uint32_t
sqsh_data_superblock_block_size(const struct SqshSuperblock *superblock);
uint32_t sqsh_data_superblock_fragment_entry_count(
		const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_compression_id(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_block_log(const struct SqshSuperblock *superblock);
uint16_t sqsh_data_superblock_flags(const struct SqshSuperblock *superblock);
uint16_t sqsh_data_superblock_id_count(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_version_major(const struct SqshSuperblock *superblock);
uint16_t
sqsh_data_superblock_version_minor(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_root_inode_ref(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_bytes_used(const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_id_table_start(const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_xattr_id_table_start(
		const struct SqshSuperblock *superblock);
uint64_t
sqsh_data_superblock_inode_table_start(const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_directory_table_start(
		const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_fragment_table_start(
		const struct SqshSuperblock *superblock);
uint64_t sqsh_data_superblock_export_table_start(
		const struct SqshSuperblock *superblock);

#endif /* end of include guard SQSH_SUPERBLOCK_DATA_H */