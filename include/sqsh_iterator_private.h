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
 * @file         sqsh_iterator_private.h
 */

#ifndef SQSH_ITERATOR_PRIVATE_H
#define SQSH_ITERATOR_PRIVATE_H

#include "sqsh_context_private.h"
#include "sqsh_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// iterator/directory_iterator.c

struct SqshDirectoryIterator {
	/**
	 * @privatesection
	 */
	struct SqshInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;

	const struct SqshDataDirectoryFragment *fragments;
	struct SqshDirectoryContext *directory;
	struct SqshMetablockStreamContext metablock;
	size_t remaining_entries;
	sqsh_index_t current_fragment_offset;
	sqsh_index_t next_offset;
	sqsh_index_t current_offset;
};

/**
 * @internal
 * @memberof SqshDirectoryIterator
 * @brief Initializes a directory iterator.
 *
 * @param[out] iterator The iterator to initialize.
 * @param[in]  inode    The inode to iterate through.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__directory_iterator_init(
		struct SqshDirectoryIterator *iterator, struct SqshInodeContext *inode);

/**
 * @internal
 * @memberof SqshDirectoryIterator
 * @brief Cleans up resources used by a directory iterator.
 *
 * @param[in] iterator The iterator to cleanup.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator);

////////////////////////////////////////
// iterator/xattr_iterator.c

struct SqshXattrIterator {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
	struct SqshMetablockStreamContext metablock;
	struct SqshMetablockStreamContext out_of_line_value;
	struct SqshXattrTable *context;
	int remaining_entries;
	sqsh_index_t next_offset;
	sqsh_index_t key_offset;
	sqsh_index_t value_offset;
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

#ifdef __cplusplus
}
#endif
#endif // SQSH_ITERATOR_PRIVATE_H
