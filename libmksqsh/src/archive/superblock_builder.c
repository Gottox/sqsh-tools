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
 * @file         superblock_builder.c
 */

#include <sqsh_data.h>
#include <sqsh_data_set.h>
#include <sqsh_error.h>

#include <sqsh_archive_builder.h>

static uint16_t
log2_u32(uint32_t x) {
	if (x == 0) {
		return UINT16_MAX;
	} else {
		return sizeof(uint32_t) * 8 - 1 - __builtin_clz(x);
	}
}

int
sqsh__superblock_builder_init(struct SqshSuperblockBuilder *superblock) {
	sqsh__data_superblock_magic_set(&superblock->data, SQSH_SUPERBLOCK_MAGIC);
	sqsh__data_superblock_version_major_set(&superblock->data, 4);
	sqsh__data_superblock_version_minor_set(&superblock->data, 0);

	return sqsh__superblock_builder_block_size(superblock, 4096);
}

int
sqsh__superblock_builder_modification_time(
		struct SqshSuperblockBuilder *superblock, uint32_t mtime) {
	sqsh__data_superblock_modification_time_set(&superblock->data, mtime);
	return 0;
}

int
sqsh__superblock_builder_block_size(
		struct SqshSuperblockBuilder *superblock, uint32_t block_size) {
	if (block_size < 4096 || block_size > 1048576) {
		return -SQSH_ERROR_BLOCKSIZE_MISMATCH;
	}
	if ((block_size & (block_size - 1)) != 0) {
		return -SQSH_ERROR_BLOCKSIZE_MISMATCH;
	}

	sqsh__data_superblock_block_size_set(&superblock->data, block_size);
	const uint16_t log2_block_size = log2_u32(block_size);
	sqsh__data_superblock_block_log_set(&superblock->data, log2_block_size);

	return 0;
}

int
sqsh__superblock_builder_fragment_count(
		struct SqshSuperblockBuilder *superblock, uint32_t fragment_count) {
	sqsh__data_superblock_fragment_entry_count_set(
			&superblock->data, fragment_count);
	return 0;
}

int
sqsh__superblock_builder_compression_id(
		struct SqshSuperblockBuilder *superblock,
		enum SqshSuperblockCompressionId compression_id) {
	sqsh__data_superblock_compression_id_set(&superblock->data, compression_id);
	return 0;
}

int
sqsh__superblock_builder_compress_inodes(
		struct SqshSuperblockBuilder *superblock, bool compress_inodes) {
	if (compress_inodes) {
		superblock->flags &= ~SQSH_SUPERBLOCK_UNCOMPRESSED_INODES;
	} else {
		superblock->flags |= SQSH_SUPERBLOCK_UNCOMPRESSED_INODES;
	}
	return 0;
}

int
sqsh__superblock_builder_compress_data(
		struct SqshSuperblockBuilder *superblock, bool compress_data) {
	if (compress_data) {
		superblock->flags &= ~SQSH_SUPERBLOCK_UNCOMPRESSED_DATA;
	} else {
		superblock->flags |= SQSH_SUPERBLOCK_UNCOMPRESSED_DATA;
	}
	return 0;
}

int
sqsh__superblock_builder_compress_fragments(
		struct SqshSuperblockBuilder *superblock, bool compress_fragments) {
	if (compress_fragments) {
		superblock->flags &= ~SQSH_SUPERBLOCK_UNCOMPRESSED_FRAGMENTS;
	} else {
		superblock->flags |= SQSH_SUPERBLOCK_UNCOMPRESSED_FRAGMENTS;
	}
	return 0;
}

int
sqsh__superblock_builder_force_fragments(
		struct SqshSuperblockBuilder *superblock, bool force_fragments) {
	if (force_fragments) {
		superblock->flags &= ~SQSH_SUPERBLOCK_ALWAYS_FRAGMENTS;
	} else {
		superblock->flags |= SQSH_SUPERBLOCK_ALWAYS_FRAGMENTS;
	}
	return 0;
}

int
sqsh__superblock_builder_deduplicate(
		struct SqshSuperblockBuilder *superblock, bool deduplicate) {
	if (deduplicate) {
		superblock->flags |= SQSH_SUPERBLOCK_DUPLICATES;
	} else {
		superblock->flags &= ~SQSH_SUPERBLOCK_DUPLICATES;
	}
	return 0;
}

int
sqsh__superblock_builder_compress_xattr(
		struct SqshSuperblockBuilder *superblock, bool compress_export) {
	if (compress_export) {
		superblock->flags &= ~SQSH_SUPERBLOCK_UNCOMPRESSED_XATTRS;
	} else {
		superblock->flags |= SQSH_SUPERBLOCK_UNCOMPRESSED_XATTRS;
	}
	return 0;
}

int
sqsh__superblock_builder_compression_options(
		struct SqshSuperblockBuilder *superblock, bool compression_options) {
	if (compression_options) {
		superblock->flags |= SQSH_SUPERBLOCK_COMPRESSOR_OPTIONS;
	} else {
		superblock->flags &= ~SQSH_SUPERBLOCK_COMPRESSOR_OPTIONS;
	}
	return 0;
}

int
sqsh__superblock_builder_id_count(
		struct SqshSuperblockBuilder *superblock, uint32_t id_count) {
	sqsh__data_superblock_id_count_set(&superblock->data, id_count);
	return 0;
}

int
sqsh__superblock_builder_root_inode_ref(
		struct SqshSuperblockBuilder *superblock, uint64_t root_inode_ref) {
	sqsh__data_superblock_root_inode_ref_set(&superblock->data, root_inode_ref);
	return 0;
}

int
sqsh__superblock_builder_inode_count(
		struct SqshSuperblockBuilder *superblock, uint32_t inode_count) {
	sqsh__data_superblock_inode_count_set(&superblock->data, inode_count);
	return 0;
}

int
sqsh__superblock_builder_bytes_used(
		struct SqshSuperblockBuilder *superblock, uint64_t bytes_used) {
	sqsh__data_superblock_bytes_used_set(&superblock->data, bytes_used);
	return 0;
}

int
sqsh__superblock_builder_id_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t id_table_start) {
	sqsh__data_superblock_id_table_start_set(&superblock->data, id_table_start);
	return 0;
}

int
sqsh__superblock_builder_xattr_table_start(
		struct SqshSuperblockBuilder *superblock,
		uint64_t xattr_id_table_start) {
	sqsh__data_superblock_xattr_id_table_start_set(
			&superblock->data, xattr_id_table_start);
	if (xattr_id_table_start == UINT64_MAX) {
		superblock->flags |= SQSH_SUPERBLOCK_NO_XATTRS;
	} else {
		superblock->flags &= ~SQSH_SUPERBLOCK_NO_XATTRS;
	}
	return 0;
}

int
sqsh__superblock_builder_inode_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t inode_table_start) {
	sqsh__data_superblock_inode_table_start_set(
			&superblock->data, inode_table_start);
	return 0;
}

int
sqsh__superblock_builder_directory_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t inode_table_start) {
	sqsh__data_superblock_directory_table_start_set(
			&superblock->data, inode_table_start);
	return 0;
}

int
sqsh__superblock_builder_fragment_table_start(
		struct SqshSuperblockBuilder *superblock,
		uint64_t export_fragment_table_start) {
	sqsh__data_superblock_fragment_table_start_set(
			&superblock->data, export_fragment_table_start);
	if (export_fragment_table_start == UINT64_MAX) {
		superblock->flags |= SQSH_SUPERBLOCK_NO_FRAGMENTS;
	} else {
		superblock->flags &= ~SQSH_SUPERBLOCK_NO_FRAGMENTS;
	}
	return 0;
}

int
sqsh__superblock_builder_export_table_start(
		struct SqshSuperblockBuilder *superblock, uint64_t export_table_start) {
	sqsh__data_superblock_export_table_start_set(
			&superblock->data, export_table_start);
	if (export_table_start == UINT64_MAX) {
		superblock->flags |= SQSH_SUPERBLOCK_EXPORTABLE;
	} else {
		superblock->flags &= ~SQSH_SUPERBLOCK_EXPORTABLE;
	}
	return 0;
}

int
sqsh__superblock_builder_write(
		struct SqshSuperblockBuilder *superblock, FILE *output) {
	int rv = 0;
	struct SqshDataSuperblock *data = &superblock->data;

	sqsh__data_superblock_flags_set(data, superblock->flags);
	rv = fwrite(data, sizeof(*data), 1, output);
	if (rv != 1) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	rv = sizeof(*data);
out:
	return rv;
}

int
sqsh__superblock_builder_cleanup(struct SqshSuperblockBuilder *superblock) {
	(void)superblock;
	return 0;
}
