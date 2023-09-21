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

#include <sqsh_archive.h>
#include <sqsh_data_private.h>
#include <sqsh_file_private.h>
#include <sqsh_table_private.h>

int
sqsh__fragment_table_init(
		struct SqshFragmentTable *table, struct SqshArchive *sqsh) {
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(sqsh);
	const uint64_t table_start =
			sqsh_superblock_fragment_table_start(superblock);
	const uint16_t count = sqsh_superblock_fragment_entry_count(superblock);

	return sqsh__table_init(
			&table->table, sqsh, table_start, sizeof(struct SqshDataFragment),
			count);
}

int
sqsh__fragment_table_get(
		const struct SqshFragmentTable *table, const struct SqshFile *file,
		struct SqshDataFragment *fragment) {
	uint32_t index = sqsh_file_fragment_block_index(file);
	return sqsh_table_get(&table->table, index, fragment);
}

int
sqsh__fragment_table_cleanup(struct SqshFragmentTable *table) {
	return sqsh__table_cleanup(&table->table);
}
