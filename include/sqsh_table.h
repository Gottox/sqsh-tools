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
 * @file         sqsh_table.h
 */

#ifndef SQSH_TABLE_H
#define SQSH_TABLE_H

#include "sqsh_mapper.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshInodeContext;

// table/table.c

struct SqshTable {
	struct Sqsh *sqsh;
	struct SqshMapper *mapper;
	struct SqshMapping lookup_table;
	uint64_t start_block;
	size_t element_size;
	size_t element_count;
};

int sqsh_table_init(
		struct SqshTable *table, struct Sqsh *sqsh, sqsh_index_t start_block,
		size_t element_size, size_t element_count);
int
sqsh_table_get(const struct SqshTable *table, sqsh_index_t index, void *target);
int sqsh_table_cleanup(struct SqshTable *table);

struct SqshFragmentTable {
	const struct SqshSuperblockContext *superblock;
	struct SqshTable table;
	struct SqshMapper *mapper;
	struct SqshCompression *compression;
};

// table/fragment_table.c

SQSH_NO_UNUSED int
sqsh_fragment_table_init(struct SqshFragmentTable *context, struct Sqsh *sqsh);

SQSH_NO_UNUSED int sqsh_fragment_table_to_buffer(
		const struct SqshFragmentTable *context,
		const struct SqshInodeContext *inode, struct SqshBuffer *buffer);

int sqsh_fragment_table_cleanup(struct SqshFragmentTable *context);

// table/xattr_table.c

struct SqshXattrLookupTable;

enum SqshXattrType {
	SQSH_XATTR_USER = 0,
	SQSH_XATTR_TRUSTED = 1,
	SQSH_XATTR_SECURITY = 2,
};

struct SqshXattrTable {
	struct Sqsh *sqsh;
	struct SqshMapping header;
	struct SqshTable table;
};

SQSH_NO_UNUSED int
sqsh_xattr_table_init(struct SqshXattrTable *context, struct Sqsh *sqsh);

uint64_t sqsh_xattr_table_start(struct SqshXattrTable *table);

int sqsh_xattr_table_get(
		const struct SqshXattrTable *table, sqsh_index_t index,
		struct SqshXattrLookupTable *target);

int sqsh_xattr_table_cleanup(struct SqshXattrTable *context);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_TABLE_H */
