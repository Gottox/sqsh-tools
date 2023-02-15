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

#ifndef SQSH_TABLE_PRIVATE_H
#define SQSH_TABLE_PRIVATE_H

#include "sqsh_mapper.h"
#include "sqsh_table.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshInodeContext;

////////////////////////////////////////
// table/table.c

struct SqshTable {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
	struct SqshMapper *mapper;
	struct SqshMapCursor lookup_table;
	uint64_t start_block;
	size_t element_size;
	size_t element_count;
};

/**
 * @internal
 * @memberof SqshTable
 * @brief Initializes a table with a SQSH context.
 *
 * @param[out] table The table to initialize.
 * @param[in]  sqsh The SQSH context to use for the table.
 * @param[in]  start_block The starting offset of the table in blocks.
 * @param[in]  element_size The size of each element in the table.
 * @param[in]  element_count The number of elements in the table.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__table_init(
		struct SqshTable *table, struct Sqsh *sqsh, sqsh_index_t start_block,
		size_t element_size, size_t element_count);

int
sqsh_table_get(const struct SqshTable *table, sqsh_index_t index, void *target);

/**
 * @internal
 * @memberof SqshTable
 * @brief Cleans up a table.
 *
 * @param[in] table The table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__table_cleanup(struct SqshTable *table);

////////////////////////////////////////
// table/fragment_table.c

struct SqshFragmentTable {
	/**
	 * @privatesection
	 */
	const struct SqshSuperblockContext *superblock;
	struct SqshTable table;
	struct SqshMapper *mapper;
	struct SqshCompression *compression;
};

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Initializes a fragment table with a SQSH context.
 *
 * @param[out] context The fragment table to initialize.
 * @param[in]  sqsh The SQSH context to use for the fragment table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh__fragment_table_init(struct SqshFragmentTable *context, struct Sqsh *sqsh);

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Cleans up a fragment table.
 *
 * @param[in] context The fragment table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__fragment_table_cleanup(struct SqshFragmentTable *context);

////////////////////////////////////////
// table/xattr_table.c

struct SqshXattrTable {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
	struct SqshMapCursor header;
	struct SqshTable table;
};

/**
 * @internal
 * @memberof SqshXattrTable
 * @brief Initializes an extended attribute table with a SQSH context.
 *
 * @param[out] context The extended attribute table to initialize.
 * @param[in]  sqsh The SQSH context to use for the table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh__xattr_table_init(struct SqshXattrTable *context, struct Sqsh *sqsh);

/**
 * @internal
 * @memberof SqshXattrTable
 * @brief Cleans up an extended attribute table.
 *
 * @param[in] context The extended attribute table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__xattr_table_cleanup(struct SqshXattrTable *context);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_TABLE_PRIVATE_H */
