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
 * @file         fragment_table.c
 */

#include "fragment_table.h"
#include "../context/inode_context.h"
#include "../context/superblock_context.h"
#include "../data/fragment_internal.h"
#include "../error.h"
#include "../hsqs.h"
#include "../mapper/mapper.h"
#include <stdint.h>

int
hsqs_fragment_table_init(struct HsqsFragmentTable *table, struct Hsqs *hsqs) {
	int rv = 0;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	uint64_t start = hsqs_superblock_fragment_table_start(superblock);
	uint32_t count = hsqs_superblock_fragment_entry_count(superblock);

	table->hsqs = hsqs;
	table->compression = hsqs_data_compression(hsqs);
	rv = hsqs_table_init(
			&table->table, hsqs, start, HSQS_SIZEOF_FRAGMENT, count);
	if (rv < 0) {
		goto out;
	}

	table->superblock = superblock;

out:
	return rv;
}

static int
read_fragment_data(
		const struct HsqsFragmentTable *table, struct HsqsBuffer *buffer,
		uint32_t index) {
	int rv = 0;
	uint64_t start;
	uint32_t size;
	const struct HsqsDatablockSize *size_info;
	bool is_compressed;
	struct HsqsFragment fragment = {0};
	struct HsqsMapping memory_map = {0};
	const uint8_t *data;

	rv = hsqs_table_get(&table->table, index, &fragment);
	if (rv < 0) {
		goto out;
	}

	start = hsqs_data_fragment_start(&fragment);
	size_info = hsqs_data_fragment_size_info(&fragment);
	size = hsqs_data_datablock_size(size_info);
	is_compressed = hsqs_data_datablock_is_compressed(size_info);

	rv = hsqs_request_map(table->hsqs, &memory_map, start, size);
	if (rv < 0) {
		goto out;
	}

	data = hsqs_mapping_data(&memory_map);
	if (is_compressed) {
		rv = hsqs_compression_decompress_to_buffer(
				table->compression, buffer, data, size);
	} else {
		rv = hsqs_buffer_append(buffer, data, size);
	}
	if (rv < 0) {
		goto out;
	}
out:
	hsqs_mapping_unmap(&memory_map);
	return rv;
}

int
hsqs_fragment_table_to_buffer(
		const struct HsqsFragmentTable *table,
		const struct HsqsInodeContext *inode, struct HsqsBuffer *buffer) {
	int rv = 0;
	struct HsqsBuffer intermediate_buffer = {0};
	const uint8_t *data;
	uint32_t block_size = hsqs_superblock_block_size(table->superblock);
	uint32_t index = hsqs_inode_file_fragment_block_index(inode);
	uint32_t offset = hsqs_inode_file_fragment_block_offset(inode);
	uint32_t size = hsqs_inode_file_size(inode) % block_size;
	uint32_t end_offset;
	if (ADD_OVERFLOW(offset, size, &end_offset)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	rv = hsqs_buffer_init(&intermediate_buffer);
	if (rv < 0) {
		goto out;
	}

	rv = read_fragment_data(table, &intermediate_buffer, index);
	if (rv < 0) {
		goto out;
	}

	if (end_offset > hsqs_buffer_size(&intermediate_buffer)) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	data = hsqs_buffer_data(&intermediate_buffer);

	rv = hsqs_buffer_append(buffer, &data[offset], size);
	if (rv < 0) {
		goto out;
	}
out:
	hsqs_buffer_cleanup(&intermediate_buffer);
	return 0;
}

int
hsqs_fragment_table_cleanup(struct HsqsFragmentTable *table) {
	hsqs_table_cleanup(&table->table);
	return 0;
}
