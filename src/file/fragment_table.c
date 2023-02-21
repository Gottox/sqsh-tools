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

#include "../../include/sqsh_table_private.h"

#include "../../include/sqsh.h"
#include "../../include/sqsh_context.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode.h"
#include "../utils.h"

#include "../../include/sqsh_compression_private.h"

// TODO: remove private header
#include "../../include/sqsh_data_private.h"

int
sqsh__fragment_table_init(struct SqshFragmentTable *table, struct Sqsh *sqsh) {
	int rv = 0;
	const struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);
	uint64_t start = sqsh_superblock_fragment_table_start(superblock);
	uint32_t count = sqsh_superblock_fragment_entry_count(superblock);

	table->compression = sqsh_compression_data(sqsh);
	table->map_manager = sqsh_map_manager(sqsh);
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
read_fragment_data(
		const struct SqshFragmentTable *table, struct SqshBuffer *buffer,
		uint32_t index) {
	int rv = 0;
	uint64_t start;
	uint32_t size;
	const struct SqshDataDatablockSize *size_info;
	bool is_compressed;
	struct SqshDataFragment fragment_info = {0};
	struct SqshMapCursor fragment_mapping = {0};
	const uint8_t *data;

	rv = sqsh_table_get(&table->table, index, &fragment_info);
	if (rv < 0) {
		goto out;
	}

	start = sqsh_data_fragment_start(&fragment_info);
	size_info = sqsh_data_fragment_size_info(&fragment_info);
	size = sqsh_data_datablock_size(size_info);
	is_compressed = sqsh_data_datablock_is_compressed(size_info);

	rv = sqsh__map_cursor_init(
			&fragment_mapping, table->map_manager, start, size);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_cursor_all(&fragment_mapping);
	if (rv < 0) {
		goto out;
	}

	data = sqsh__map_cursor_data(&fragment_mapping);
	if (is_compressed) {
		rv = sqsh__compression_decompress_to_buffer(
				table->compression, buffer, data, size);
	} else {
		rv = sqsh__buffer_append(buffer, data, size);
	}
	if (rv < 0) {
		goto out;
	}
out:
	sqsh__map_cursor_cleanup(&fragment_mapping);
	return rv;
}

int
sqsh_fragment_table_to_buffer(
		const struct SqshFragmentTable *table,
		const struct SqshInodeContext *inode, struct SqshBuffer *buffer) {
	int rv = 0;
	struct SqshBuffer intermediate_buffer = {0};
	const uint8_t *data;
	uint32_t block_size = sqsh_superblock_block_size(table->superblock);
	uint32_t index = sqsh_inode_file_fragment_block_index(inode);
	uint32_t offset = sqsh_inode_file_fragment_block_offset(inode);
	uint32_t size = sqsh_inode_file_size(inode) % block_size;
	uint32_t end_offset;
	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	rv = sqsh__buffer_init(&intermediate_buffer);
	if (rv < 0) {
		goto out;
	}

	rv = read_fragment_data(table, &intermediate_buffer, index);
	if (rv < 0) {
		goto out;
	}

	if (end_offset > sqsh__buffer_size(&intermediate_buffer)) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}

	data = sqsh__buffer_data(&intermediate_buffer);

	rv = sqsh__buffer_append(buffer, &data[offset], size);
	if (rv < 0) {
		goto out;
	}
out:
	sqsh__buffer_cleanup(&intermediate_buffer);
	return 0;
}

int
sqsh__fragment_table_cleanup(struct SqshFragmentTable *table) {
	sqsh_table_cleanup(&table->table);
	return 0;
}
