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
 * @file         sqsh_archive_builder.h
 */

#ifndef SQSH_ARCHIVE_BUILDER_H
#define SQSH_ARCHIVE_BUILDER_H

#include <cextras/collection.h>
#include <sqsh_data.h>
#include <sqsh_data_set.h>

#include <stdio.h>

/***************************************
 * archive/superblock_builder.c
 */

struct SqshSuperblockBuilder {
	struct SqshDataSuperblock data;
	uint16_t flags;
};

int sqsh__superblock_builder_init(struct SqshSuperblockBuilder *superblock);

int sqsh__superblock_builder_modification_time(
		struct SqshSuperblockBuilder *superblock, uint32_t mtime);

int sqsh__superblock_builder_block_size(
		struct SqshSuperblockBuilder *superblock, uint32_t block_size);

int sqsh__superblock_builder_fragment_count(
		struct SqshSuperblockBuilder *superblock, uint32_t fragment_count);

int sqsh__superblock_builder_compression_id(
		struct SqshSuperblockBuilder *superblock,
		enum SqshSuperblockCompressionId compression_id);

int sqsh__superblock_builder_compress_inodes(
		struct SqshSuperblockBuilder *superblock, bool compress_inodes);

int sqsh__superblock_builder_compress_data(
		struct SqshSuperblockBuilder *superblock, bool compress_data);

int sqsh__superblock_builder_compress_fragments(
		struct SqshSuperblockBuilder *superblock, bool compress_fragments);

int sqsh__superblock_builder_force_fragments(
		struct SqshSuperblockBuilder *superblock, bool force_fragments);

int sqsh__superblock_builder_deduplicate(
		struct SqshSuperblockBuilder *superblock, bool deduplicate);

int sqsh__superblock_builder_compress_xattr(
		struct SqshSuperblockBuilder *superblock, bool compress_export);

int sqsh__superblock_builder_compression_options(
		struct SqshSuperblockBuilder *superblock, bool compression_options);

int sqsh__superblock_builder_id_count(
		struct SqshSuperblockBuilder *superblock, uint32_t id_count);

int sqsh__superblock_builder_root_inode_ref(
		struct SqshSuperblockBuilder *superblock, uint64_t root_inode_ref);

int sqsh__superblock_builder_inode_count(
		struct SqshSuperblockBuilder *superblock, uint32_t inode_count);

int sqsh__superblock_builder_bytes_used(
		struct SqshSuperblockBuilder *superblock, uint64_t bytes_used);

int sqsh__superblock_builder_id_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t id_table_start);

int sqsh__superblock_builder_xattr_table_start(
		struct SqshSuperblockBuilder *superblock,
		uint64_t xattr_id_table_start);

int sqsh__superblock_builder_inode_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t inode_table_start);

int sqsh__superblock_builder_directory_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t inode_table_start);

int sqsh__superblock_builder_fragment_table_start(
		struct SqshSuperblockBuilder *superblock,
		uint64_t export_fragment_table_start);

int sqsh__superblock_builder_export_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t export_table_start);

int sqsh__superblock_builder_fragment_table(
		struct SqshSuperblockBuilder *superblock,
		uint64_t fragment_table_start);

int sqsh__superblock_builder_write(
		struct SqshSuperblockBuilder *superblock, FILE *output);

int sqsh__superblock_builder_cleanup(struct SqshSuperblockBuilder *superblock);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ARCHIVE_BUILDER_H */
