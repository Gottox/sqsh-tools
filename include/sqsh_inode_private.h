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
 * @file         sqsh_inode_private.h
 */

#ifndef SQSH_INODE_PRIVATE_H
#define SQSH_INODE_PRIVATE_H

#include "sqsh_context_private.h"
#include "sqsh_inode.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// context/inode_context.c

struct SqshInodeContext {
	/**
	 * @privatesection
	 */
	struct SqshMetablockStreamContext metablock;
	struct Sqsh *sqsh;
};

/**
 * @internal
 * @brief Initialize the inode context from a inode reference. inode references
 * @memberof SqshInodeContext
 * are descriptors of the physical location of an inode inside the inode table.
 * They are diffrent from the inode number. In doubt use the inode number.
 *
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param inode_ref The inode reference.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__inode_init(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_ref);
/**
 * @internal
 * @brief cleans up the inode context.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return int 0 on success, less than 0 on error.
 */
int sqsh__inode_cleanup(struct SqshInodeContext *context);

////////////////////////////////////////
// inode/directory_index_iterator.c

struct SqshDirectoryIndexIterator {
	/**
	 * @privatesection
	 */
	struct SqshInodeContext *inode;
	size_t remaining_entries;
	sqsh_index_t current_offset;
	sqsh_index_t next_offset;
};

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Initializes an iterator for a directory index
 *
 * @param[in] iterator The iterator to initialize
 * @param[in] inode    The inode for the directory to iterate over
 *
 * @return 0 on success, negative value on error
 */
SQSH_NO_UNUSED int sqsh__directory_index_iterator_init(
		struct SqshDirectoryIndexIterator *iterator,
		struct SqshInodeContext *inode);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Creates a new iterator for a directory index
 *
 * @param[in] inode The inode for the directory to iterate over
 * @param[out] err  Pointer to an int where the error code will be stored
 *
 * @return The newly created iterator on success, NULL on error
 */
SQSH_NO_UNUSED
struct SqshDirectoryIndexIterator *
sqsh__directory_index_iterator_new(struct SqshInodeContext *inode, int *err);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Advances the iterator to the next entry in the directory index
 *
 * @param[in] iterator The iterator to advance
 *
 * @return 0 on success, negative value on error
 */
SQSH_NO_UNUSED int sqsh__directory_index_iterator_next(
		struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Gets the index of the current entry in the directory index
 *
 * @param[in] iterator The iterator to get the index from
 *
 * @return The index of the current entry
 */
uint32_t sqsh__directory_index_iterator_index(
		const struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Gets the start offset of the current entry in the directory index
 *
 * @param[in] iterator The iterator to get the start offset from
 *
 * @return The start offset of the current entry
 */
uint32_t sqsh__directory_index_iterator_start(
		const struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Gets the name size of the current entry in the directory index
 *
 * @param[in] iterator The iterator to get the name size from
 *
 * @return The name size of the current entry
 */
uint32_t sqsh__directory_index_iterator_name_size(
		const struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Gets the name of the current entry in the directory index
 *
 * @param[in] iterator The iterator to get the name from
 *
 * @return The name of the current entry
 */
const char *sqsh__directory_index_iterator_name(
		const struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Cleans up an iterator for a directory index
 *
 * @param[in] iterator The iterator to clean up
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__directory_index_iterator_cleanup(
		struct SqshDirectoryIndexIterator *iterator);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Frees the resources used by the given directory index iterator.
 *
 * @param iterator The iterator to free.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__directory_index_iterator_free(
		struct SqshDirectoryIndexIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_INODE_PRIVATE_H */
