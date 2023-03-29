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
 * @file         fragment_table.c
 */

#include "../../include/sqsh_file_private.h"

#include "../../include/sqsh_inode.h"
#include "../utils.h"

// TODO: remove private header
#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data_private.h"

static int
init_compression_manager(
		struct SqshFragmentTable *table, struct SqshArchive *archive) {
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const struct SqshCompression *compression =
			sqsh_archive_compression_data(archive);

	// we ignore the fact, that the datablock may begin at a different
	// address when compression options are present.
	uint64_t start_address = SQSH_SIZEOF_SUPERBLOCK;
	const uint64_t upper_limit = sqsh_superblock_inode_table_start(superblock);

	table->map_manager = sqsh_archive_map_manager(archive);
	// TODO: Is it safe to assume, that every fragment has at least 2 entries?
	// (except when ther is only one packed file)
	// Be safe and assume it is not for now:
	size_t size = sqsh_superblock_fragment_entry_count(superblock);
	return sqsh__compression_manager_init(
			&table->compression_manager, archive, compression, start_address,
			upper_limit, size);
}

int
sqsh__fragment_table_init(
		struct SqshFragmentTable *table, struct SqshArchive *sqsh) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(sqsh);
	uint64_t start = sqsh_superblock_fragment_table_start(superblock);
	uint32_t count = sqsh_superblock_fragment_entry_count(superblock);

	rv = init_compression_manager(table, sqsh);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__table_init(
			&table->table, sqsh, start, SQSH_SIZEOF_FRAGMENT, count);
	if (rv < 0) {
		goto out;
	}

	table->superblock = superblock;

out:
	return rv;
}

static int
append_fragment(
		struct SqshFragmentTable *table, const struct SqshInode *inode,
		struct SqshBuffer *buffer, const uint8_t *data, size_t data_size) {
	uint32_t block_size = sqsh_superblock_block_size(table->superblock);
	uint32_t offset = sqsh_inode_file_fragment_block_offset(inode);
	uint32_t size = sqsh_inode_file_size(inode) % block_size;
	uint32_t end_offset;
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (end_offset > data_size) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}

	int rv = 0;
	rv = sqsh__buffer_append(buffer, &data[offset], size);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

static int
read_fragment_compressed(
		struct SqshFragmentTable *table, const struct SqshInode *inode,
		struct SqshBuffer *buffer, uint64_t start_address, uint32_t size) {
	int rv = 0;
	const struct SqshBuffer *uncompressed = NULL;
	rv = sqsh__compression_manager_get(
			&table->compression_manager, start_address, size, &uncompressed);
	if (rv < 0) {
		goto out;
	}
	const uint8_t *data = sqsh__buffer_data(uncompressed);
	const size_t data_size = sqsh__buffer_size(uncompressed);

	rv = append_fragment(table, inode, buffer, data, data_size);

	if (rv < 0) {
		goto out;
	}

out:
	sqsh__compression_manager_release(
			&table->compression_manager, uncompressed);
	return rv;
}

static int
read_fragment_uncompressed(
		struct SqshFragmentTable *table, const struct SqshInode *inode,
		struct SqshBuffer *buffer, uint64_t start_address, uint32_t size) {
	int rv = 0;
	const uint8_t *data;
	uint64_t upper_limit;
	struct SqshMapReader reader = {0};

	if (SQSH_ADD_OVERFLOW(start_address, size, &upper_limit)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	rv = sqsh__map_reader_init(
			&reader, table->map_manager, start_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_reader_all(&reader);
	if (rv < 0) {
		goto out;
	}
	data = sqsh__map_reader_data(&reader);
	rv = append_fragment(table, inode, buffer, data, size);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__map_reader_cleanup(&reader);
	return rv;
}

int
sqsh_fragment_table_to_buffer(
		struct SqshFragmentTable *table, const struct SqshInode *inode,
		struct SqshBuffer *buffer) {
	uint32_t index = sqsh_inode_file_fragment_block_index(inode);

	int rv = 0;
	struct SqshDataFragment fragment_info = {0};

	rv = sqsh_table_get(&table->table, index, &fragment_info);
	if (rv < 0) {
		goto out;
	}

	const uint64_t start_address = sqsh_data_fragment_start(&fragment_info);
	const struct SqshDataDatablockSize *size_info =
			sqsh_data_fragment_size_info(&fragment_info);
	const uint32_t size = sqsh_data_datablock_size(size_info);
	const bool is_compressed = sqsh_data_datablock_is_compressed(size_info);

	if (is_compressed) {
		rv = read_fragment_compressed(
				table, inode, buffer, start_address, size);
	} else {
		rv = read_fragment_uncompressed(
				table, inode, buffer, start_address, size);
	}
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

int
sqsh__fragment_table_cleanup(struct SqshFragmentTable *table) {
	sqsh__table_cleanup(&table->table);
	sqsh__compression_manager_cleanup(&table->compression_manager);
	return 0;
}
