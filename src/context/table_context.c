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
#include "superblock_context.h"
#include <stdint.h>

static const uint64_t *
lookup_table(struct HsqsTableContext *table) {
	return (const uint64_t *)hsqs_map_data(&table->lookup_table);
}

int
hsqs_table_init(
		struct HsqsTableContext *table,
		const struct HsqsSuperblockContext *superblock, off_t start_block,
		size_t element_size, size_t element_count) {
	int rv = 0;
	size_t byte_size;
	uint_fast64_t table_upper_limit;
	struct HsqsMemoryMapper *mapper = superblock->mapper;
	size_t size = hsqs_divide_ceil_u32(
			element_size * element_count, HSQS_METABLOCK_BLOCK_SIZE);

	// Make sure the start block is at least big enough to hold one entry.
	if (ADD_OVERFLOW(start_block, sizeof(uint64_t), &table_upper_limit)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	if (hsqs_mapper_size(superblock->mapper) < table_upper_limit) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	rv = hsqs_mapper_map(&table->lookup_table, mapper, start_block, size);
	if (rv < 0) {
		return rv;
	}

	rv = hsqs_metablock_init(
			&table->metablock, superblock, lookup_table(table)[0]);
	if (rv < 0) {
		goto out;
	}

	table->element_size = element_size;
	table->element_count = element_count;
	if (MULT_OVERFLOW(element_size, element_count, &byte_size)) {
		rv = -HSQS_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = hsqs_metablock_more(&table->metablock, byte_size);

out:
	return rv;
}

int
hsqs_table_get(
		struct HsqsTableContext *table, off_t index, const void **target) {
	off_t offset;

	if (MULT_OVERFLOW(index, table->element_size, &offset)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	*target = &hsqs_metablock_data(&table->metablock)[offset];

	return 0;
}

int
hsqs_table_cleanup(struct HsqsTableContext *table) {
	hsqs_metablock_cleanup(&table->metablock);
	return 0;
}
