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

struct MksqshSuperblock {
	struct SqshDataSuperblock data;
	uint16_t flags;
};

int mksqsh__superblock_init(struct MksqshSuperblock *superblock);

int mksqsh__superblock_modification_time(
		struct MksqshSuperblock *superblock, uint32_t mtime);

int mksqsh__superblock_block_size(
		struct MksqshSuperblock *superblock, uint32_t block_size);

int mksqsh__superblock_fragment_count(
		struct MksqshSuperblock *superblock, uint32_t fragment_count);

int mksqsh__superblock_compression_id(
		struct MksqshSuperblock *superblock,
		enum SqshSuperblockCompressionId compression_id);

int mksqsh__superblock_compress_inodes(
		struct MksqshSuperblock *superblock, bool compress_inodes);

int mksqsh__superblock_compress_data(
		struct MksqshSuperblock *superblock, bool compress_data);

int mksqsh__superblock_compress_fragments(
		struct MksqshSuperblock *superblock, bool compress_fragments);

int mksqsh__superblock_force_fragments(
		struct MksqshSuperblock *superblock, bool force_fragments);

int mksqsh__superblock_deduplicate(
		struct MksqshSuperblock *superblock, bool deduplicate);

int mksqsh__superblock_compress_xattr(
		struct MksqshSuperblock *superblock, bool compress_export);

int mksqsh__superblock_compression_options(
		struct MksqshSuperblock *superblock, bool compression_options);

int mksqsh__superblock_id_count(
		struct MksqshSuperblock *superblock, uint32_t id_count);

int mksqsh__superblock_root_inode_ref(
		struct MksqshSuperblock *superblock, uint64_t root_inode_ref);

int mksqsh__superblock_inode_count(
		struct MksqshSuperblock *superblock, uint32_t inode_count);

int mksqsh__superblock_bytes_used(
		struct MksqshSuperblock *superblock, uint64_t bytes_used);

int mksqsh__superblock_id_table_start(
		struct MksqshSuperblock *superblock, uint64_t id_table_start);

int mksqsh__superblock_xattr_table_start(
		struct MksqshSuperblock *superblock, uint64_t xattr_id_table_start);

int mksqsh__superblock_inode_table_start(
		struct MksqshSuperblock *superblock, uint64_t inode_table_start);

int mksqsh__superblock_directory_table_start(
		struct MksqshSuperblock *superblock, uint64_t inode_table_start);

int mksqsh__superblock_fragment_table_start(
		struct MksqshSuperblock *superblock,
		uint64_t export_fragment_table_start);

int mksqsh__superblock_export_table_start(
		struct MksqshSuperblock *superblock, uint64_t export_table_start);

int mksqsh__superblock_write(struct MksqshSuperblock *superblock, FILE *output);

int mksqsh__superblock_cleanup(struct MksqshSuperblock *superblock);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ARCHIVE_BUILDER_H */
