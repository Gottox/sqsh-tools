/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         table.c
 */

#include <sqsh_table_private.h>

#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <sqsh_metablock_private.h>

typedef const __attribute__((aligned(1))) uint64_t unaligned_uint64_t;

static uint64_t
lookup_table_get(const struct SqshTable *table, sqsh_index_t index) {
	unaligned_uint64_t *lookup_table =
			(unaligned_uint64_t *)sqsh__map_reader_data(&table->lookup_table);

	return lookup_table[index];
}

int
sqsh__table_init(
		struct SqshTable *table, struct SqshArchive *sqsh, uint64_t start_block,
		size_t element_size, size_t element_count) {
	int rv = 0;
	size_t byte_size;
	size_t table_size;
	size_t lookup_table_size;
	size_t lookup_table_count;
	uint64_t upper_limit;
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(sqsh);

	if (SQSH_MULT_OVERFLOW(element_size, element_count, &table_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	lookup_table_count =
			SQSH_DIVIDE_CEIL(table_size, SQSH_METABLOCK_BLOCK_SIZE);

	if (SQSH_MULT_OVERFLOW(
				lookup_table_count, sizeof(uint64_t), &lookup_table_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	if (SQSH_ADD_OVERFLOW(start_block, lookup_table_size, &upper_limit)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	rv = sqsh__map_reader_init(
			&table->lookup_table, map_manager, start_block, upper_limit);
	if (rv < 0) {
		return rv;
	}
	rv = sqsh__map_reader_all(&table->lookup_table);
	if (rv < 0) {
		goto out;
	}

	table->sqsh = sqsh;
	table->element_size = element_size;
	table->element_count = element_count;
	if (SQSH_MULT_OVERFLOW(element_size, element_count, &byte_size)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

out:
	if (rv < 0) {
		sqsh__table_cleanup(table);
	}
	return rv;
}

int
sqsh_table_get(
		const struct SqshTable *table, sqsh_index_t index, void *target) {
	int rv = 0;
	struct SqshArchive *sqsh = table->sqsh;
	struct SqshMetablockReader metablock = {0};
	size_t lookup_table_bytes = sqsh__map_reader_size(&table->lookup_table);
	sqsh_index_t lookup_index =
			index * table->element_size / SQSH_METABLOCK_BLOCK_SIZE;
	if (lookup_index >= table->element_count ||
		lookup_index * sizeof(uint64_t) >= lookup_table_bytes) {
		return -SQSH_ERROR_OUT_OF_BOUNDS;
	}
	uint64_t metablock_address = lookup_table_get(table, lookup_index);
	uint64_t element_offset =
			(index * table->element_size) % SQSH_METABLOCK_BLOCK_SIZE;

	uint64_t upper_limit = SQSH_METABLOCK_BLOCK_SIZE;
	if (SQSH_ADD_OVERFLOW(metablock_address, upper_limit, &upper_limit)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	rv = sqsh__metablock_reader_init(
			&metablock, sqsh, metablock_address, upper_limit);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__metablock_reader_advance(
			&metablock, element_offset, table->element_size);
	if (rv < 0) {
		goto out;
	}

	memcpy(target, sqsh__metablock_reader_data(&metablock),
		   table->element_size);

out:
	sqsh__metablock_reader_cleanup(&metablock);
	return rv;
}

int
sqsh__table_cleanup(struct SqshTable *table) {
	sqsh__map_reader_cleanup(&table->lookup_table);
	return 0;
}
