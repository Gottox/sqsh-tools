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
 * @file         sqsh_xattr_private.h
 */

#ifndef SQSH_XATTR_PRIVATE_H
#define SQSH_XATTR_PRIVATE_H

#include "sqsh_context_private.h"
#include "sqsh_table_private.h"
#include "sqsh_xattr.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// xattr/xattr_iterator.c

struct SqshXattrIterator {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
	struct SqshMetablockCursor metablock;
	struct SqshMetablockCursor out_of_line_value;
	struct SqshXattrTable *context;
	size_t remaining_entries;
	uint64_t upper_limit;
	sqsh_index_t next_offset;
	sqsh_index_t value_index;
};

/**
 * @internal
 * @memberof SqshXattrIterator
 * @brief Initializes a new xattr iterator.
 *
 * @param[out] iterator The iterator to initialize.
 * @param[in]  inode    The inode to iterate through xattrs.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__xattr_iterator_init(
		struct SqshXattrIterator *iterator,
		const struct SqshInodeContext *inode);

/**
 * @internal
 * @memberof SqshXattrIterator
 * @brief Cleans up resources used by an xattr iterator.
 *
 * @param[in] iterator The iterator to cleanup.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__xattr_iterator_cleanup(struct SqshXattrIterator *iterator);

////////////////////////////////////////
// xattr/xattr_table.c

struct SqshXattrTable {
	/**
	 * @privatesection
	 */
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
#endif // SQSH_XATTR_PRIVATE_H
