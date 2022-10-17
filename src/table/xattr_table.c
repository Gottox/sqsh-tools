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
 * @file         xattr_table.c
 */

#include "xattr_table.h"
#include "../context/inode_context.h"
#include "../context/superblock_context.h"
#include "../data/xattr.h"
#include "../error.h"
#include "../primitive/buffer.h"
#include "../sqsh.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const struct SqshXattrIdTable *
get_header(const struct SqshXattrTable *context) {
	return (struct SqshXattrIdTable *)sqsh_mapping_data(&context->header);
}

int
sqsh_xattr_table_init(struct SqshXattrTable *context, struct Sqsh *sqsh) {
	int rv = 0;
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);
	struct SqshMapper *mapper = sqsh_mapper(sqsh);
	uint64_t xattr_address = sqsh_superblock_xattr_id_table_start(superblock);
	uint64_t bytes_used = sqsh_superblock_bytes_used(superblock);
	if (xattr_address + SQSH_SIZEOF_XATTR_ID_TABLE >= bytes_used) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}
	context->sqsh = sqsh;
	rv = sqsh_mapper_map(
			&context->header, mapper, xattr_address,
			SQSH_SIZEOF_XATTR_ID_TABLE);
	if (rv < 0) {
		goto out;
	}

	const struct SqshXattrIdTable *header = get_header(context);

	rv = sqsh_table_init(
			&context->table, sqsh, xattr_address + SQSH_SIZEOF_XATTR_ID_TABLE,
			SQSH_SIZEOF_XATTR_LOOKUP_TABLE,
			sqsh_data_xattr_id_table_xattr_ids(header));
	if (rv < 0) {
		goto out;
	}
out:
	if (rv < 0) {
		sqsh_xattr_table_cleanup(context);
	}
	return rv;
}

uint64_t
sqsh_xattr_table_start(struct SqshXattrTable *table) {
	const struct SqshXattrIdTable *header = get_header(table);
	return sqsh_data_xattr_id_table_xattr_table_start(header);
}

int
sqsh_xattr_table_cleanup(struct SqshXattrTable *context) {
	sqsh_table_cleanup(&context->table);
	sqsh_mapping_unmap(&context->header);
	return 0;
}
