/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : table_context
 * @created     : Sunday Sep 26, 2021 19:11:49 CEST
 */

#include "table_context.h"
#include "../error.h"
#include "metablock_context.h"
#include "superblock_context.h"
#include <stdint.h>
#include <string.h>

static uint64_t
lookup_table_get(const struct HsqsTableContext *table, off_t index) {
	const uint64_t *lookup_table =
			(const uint64_t *)hsqs_map_data(&table->lookup_table);

	return lookup_table[index];
}

int
hsqs_table_init(
		struct HsqsTableContext *table,
		const struct HsqsSuperblockContext *superblock, off_t start_block,
		size_t element_size, size_t element_count) {
	int rv = 0;
	size_t byte_size;
	struct HsqsMapper *mapper = superblock->mapper;
	// TODO: Overflow
	size_t lookup_table_size =
			HSQS_DEVIDE_CEIL(
					element_size * element_count, HSQS_METABLOCK_BLOCK_SIZE) *
			sizeof(uint64_t);

	rv = hsqs_mapper_map(
			&table->lookup_table, mapper, start_block, lookup_table_size);
	if (rv < 0) {
		return rv;
	}

	table->mapper = superblock->mapper;
	table->superblock = superblock;
	table->element_size = element_size;
	table->element_count = element_count;
	if (MULT_OVERFLOW(element_size, element_count, &byte_size)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

out:
	return rv;
}

int
hsqs_table_get(
		const struct HsqsTableContext *table, off_t index, void *target) {
	int rv = 0;
	struct HsqsMetablockContext metablock = {0};
	struct HsqsBuffer buffer = {0};
	uint64_t lookup_index =
			index * table->element_size / HSQS_METABLOCK_BLOCK_SIZE;
	uint64_t metablock_address = lookup_table_get(table, lookup_index);
	uint64_t element_index =
			(index * table->element_size) % HSQS_METABLOCK_BLOCK_SIZE;

	enum HsqsSuperblockCompressionId compression_id =
			hsqs_superblock_compression_id(table->superblock);
	rv = hsqs_buffer_init(&buffer, compression_id, HSQS_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_metablock_init(&metablock, table->superblock, metablock_address);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_metablock_to_buffer(&metablock, &buffer);
	if (rv < 0) {
		goto out;
	}

	memcpy(target, &hsqs_buffer_data(&buffer)[element_index],
		   table->element_size);

out:
	hsqs_metablock_cleanup(&metablock);
	hsqs_buffer_cleanup(&buffer);
	return rv;
}

int
hsqs_table_cleanup(struct HsqsTableContext *table) {
	hsqs_map_unmap(&table->lookup_table);
	return 0;
}
