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
 * @file         xattr_table.c
 */

#include "../../include/sqsh_xattr_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

static const struct SqshDataXattrIdTable *
get_header(const struct SqshXattrTable *context) {
	return (struct SqshDataXattrIdTable *)sqsh__map_reader_data(
			&context->header);
}

int
sqsh__xattr_table_init(
		struct SqshXattrTable *context, struct SqshArchive *sqsh) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(sqsh);
	struct SqshMapManager *map_manager = sqsh_archive_map_manager(sqsh);
	const uint64_t xattr_address =
			sqsh_superblock_xattr_id_table_start(superblock);
	uint64_t upper_limit;
	if (SQSH_ADD_OVERFLOW(
				xattr_address, SQSH_SIZEOF_XATTR_ID_TABLE, &upper_limit)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	rv = sqsh__map_reader_init(
			&context->header, map_manager, xattr_address, upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__map_reader_all(&context->header);
	if (rv < 0) {
		goto out;
	}

	const struct SqshDataXattrIdTable *header = get_header(context);

	// TODO: sqsh__table_init currently does the mapping of the table
	// itself. This results in mapping the table and the header. They
	// should be mapped once.
	rv = sqsh__table_init(
			&context->table, sqsh, xattr_address + SQSH_SIZEOF_XATTR_ID_TABLE,
			SQSH_SIZEOF_XATTR_LOOKUP_TABLE,
			sqsh_data_xattr_id_table_xattr_ids(header));
	if (rv < 0) {
		goto out;
	}
out:
	if (rv < 0) {
		sqsh__xattr_table_cleanup(context);
	}
	return rv;
}

uint64_t
sqsh_xattr_table_start(struct SqshXattrTable *table) {
	const struct SqshDataXattrIdTable *header = get_header(table);
	return sqsh_data_xattr_id_table_xattr_table_start(header);
}

int
sqsh_xattr_table_get(
		const struct SqshXattrTable *table, sqsh_index_t index,
		struct SqshDataXattrLookupTable *target) {
	return sqsh_table_get(&table->table, index, target);
}

int
sqsh__xattr_table_cleanup(struct SqshXattrTable *context) {
	sqsh_table_cleanup(&context->table);
	sqsh__map_reader_cleanup(&context->header);
	return 0;
}
