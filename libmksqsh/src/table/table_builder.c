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
 * @file         inode_builder.c
 */

#define _DEFAULT_SOURCE

#include <assert.h>
#include <cextras/endian.h>
#include <mksqsh_table.h>
#include <sqsh_common_private.h>
#include <sqsh_data_set.h>

int
mksqsh__table_init(
		struct MksqshTable *table, size_t entry_size, FILE *metablock_output,
		FILE *output) {
	assert(0 == entry_size % SQSH_METABLOCK_BLOCK_SIZE);

	table->entry_size = entry_size;
	table->output = output;
	table->metablock_output = metablock_output;
	return mksqsh__metablock_init(&table->metablock_writer, metablock_output);
}

int
mksqsh__table_add(
		struct MksqshTable *table, const void *entry, size_t entry_size) {
	int rv = 0;
	(void)table;
	(void)entry;
	(void)entry_size;
	assert(entry_size == table->entry_size);

	const uint64_t ref = mksqsh__metablock_ref(&table->metablock_writer);
	const uint64_t outer_ref = sqsh_address_ref_outer_offset(ref);
	const uint64_t inner_ref =
			sqsh_address_ref_inner_offset(ref) / table->entry_size;

	const uint64_t current_ref = sqsh_address_ref_create(outer_ref, inner_ref);
	table->entry_count += 1;

	const uint64_t current_ref_le = CX_CPU_2_LE64(current_ref);
	const unsigned long write =
			fwrite(&current_ref_le, sizeof(current_ref_le), 1, table->output);
	if (write != 1) {
		rv = -1; // TODO: proper error code
		goto out;
	}

	rv = mksqsh__metablock_write(&table->metablock_writer, entry, entry_size);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

int
mksqsh__table_flush(struct MksqshTable *table) {
	int rv = 0;
	FILE *metablock_output = table->metablock_output;
	FILE *output = table->output;

	rv = mksqsh__metablock_flush(&table->metablock_writer);
	if (rv < 0) {
		goto out;
	}

	rv = fseek(metablock_output, 0, SEEK_SET);
	if (rv < 0) {
		goto out;
	}

	for (;;) {
		char buffer[BUFSIZ];
		const unsigned long read =
				fread(buffer, 1, sizeof(buffer), metablock_output);
		if (read == 0) {
			if (!feof(table->metablock_output)) {
				rv = -1; // TODO: proper error code
				goto out;
			}
			break;
		}
		const unsigned long write = fwrite(buffer, 1, read, output);
		if (write != read) {
			rv = -1; // TODO: proper error code
			goto out;
		}
	}
out:
	return rv;
}

int
mksqsh__table_cleanup(struct MksqshTable *table) {
	return mksqsh__metablock_cleanup(&table->metablock_writer);
}
