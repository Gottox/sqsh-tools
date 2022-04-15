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
 * @file        : table
 * @created     : Sunday Sep 26, 2021 19:11:49 CEST
 */

#include "table.h"
#include "../context/metablock_context.h"
#include "../error.h"
#include "../hsqs.h"
#include <stdint.h>
#include <string.h>

typedef const __attribute__((aligned(1))) uint64_t unaligned_uint64_t;
static uint64_t
lookup_table_get(const struct HsqsTable *table, off_t index) {
	unaligned_uint64_t *lookup_table =
			(unaligned_uint64_t *)hsqs_mapping_data(&table->lookup_table);

	return lookup_table[index];
}

int
hsqs_table_init(
		struct HsqsTable *table, struct Hsqs *hsqs, off_t start_block,
		size_t element_size, size_t element_count) {
	int rv = 0;
	size_t byte_size;
	size_t table_size;
	size_t lookup_table_size;
	size_t lookup_table_count;

	if (MULT_OVERFLOW(element_size, element_count, &table_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	lookup_table_count =
			HSQS_DEVIDE_CEIL(table_size, HSQS_METABLOCK_BLOCK_SIZE);

	if (MULT_OVERFLOW(
				lookup_table_count, sizeof(uint64_t), &lookup_table_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	rv = hsqs_request_map(
			hsqs, &table->lookup_table, start_block, lookup_table_size);
	if (rv < 0) {
		return rv;
	}

	table->hsqs = hsqs;
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
hsqs_table_get(const struct HsqsTable *table, off_t index, void *target) {
	int rv = 0;
	struct Hsqs *hsqs = table->hsqs;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	struct HsqsMetablockContext metablock = {0};
	struct HsqsBuffer buffer = {0};
	uint64_t lookup_index =
			index * table->element_size / HSQS_METABLOCK_BLOCK_SIZE;
	uint64_t metablock_address = lookup_table_get(table, lookup_index);
	uint64_t element_index =
			(index * table->element_size) % HSQS_METABLOCK_BLOCK_SIZE;

	enum HsqsSuperblockCompressionId compression_id =
			hsqs_superblock_compression_id(superblock);
	rv = hsqs_buffer_init(&buffer, compression_id, HSQS_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_metablock_init(&metablock, hsqs, metablock_address);
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
hsqs_table_cleanup(struct HsqsTable *table) {
	hsqs_mapping_unmap(&table->lookup_table);
	return 0;
}
