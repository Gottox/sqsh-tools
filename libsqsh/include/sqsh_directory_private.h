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
 * @file         sqsh_directory_private.h
 */

#ifndef SQSH_ITERATOR_PRIVATE_H
#define SQSH_ITERATOR_PRIVATE_H

#include "sqsh_directory.h"

#include "sqsh_file_private.h"
#include "sqsh_metablock_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * directory/directory_iterator.c
 */

/**
 * @brief A directory iterator.
 */
struct SqshDirectoryIterator {
	/**
	 * @privatesection
	 */
	const struct SqshFile *file;
	uint32_t remaining_size;

	struct SqshMetablockReader metablock;
	size_t remaining_entries;
	sqsh_index_t next_offset;
	uint32_t current_inode;

	uint32_t start_base;
	uint32_t inode_base;
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
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__directory_iterator_init(
		struct SqshDirectoryIterator *iterator, const struct SqshFile *inode);

/**
 * @internal
 * @memberof SqshDirectoryIterator
 * @brief Cleans up resources used by a directory iterator.
 *
 * @param[in] iterator The iterator to cleanup.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int
sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator);

/***************************************
 * inode/directory_index_iterator.c
 */

/**
 * @brief Iterator for directory indexes
 */
struct SqshDirectoryIndexIterator {
	/**
	 * @privatesection
	 */
	struct SqshFile file;
	size_t remaining_entries;
	sqsh_index_t next_offset;
};

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 *
 * @brief Initializes an iterator for a directory index
 *
 * @param[out] iterator  The iterator to initialize
 * @param[in] sqsh      The sqsh context
 * @param[in] inode_ref The inode ref for the directory to iterate over
 *
 * @return 0 on success, negative value on error
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__directory_index_iterator_init(
		struct SqshDirectoryIndexIterator *iterator, struct SqshArchive *sqsh,
		uint64_t inode_ref);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Advances the iterator to the next entry in the directory index
 *
 * @param[in]  iterator  The iterator to advance
 * @param[out] err       Pointer to an int where the error code will be stored.
 *
 * @retval true  When the iterator was advanced
 * @retval false When the iterator is at the end and no more entries are
 * available or if an error occured.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED bool sqsh__directory_index_iterator_next(
		struct SqshDirectoryIndexIterator *iterator, int *err);

/**
 * @internal
 * @memberof SqshDirectoryIndexIterator
 * @brief Gets the index of the current entry in the directory index
 *
 * @param[in] iterator The iterator to get the index from
 *
 * @return The index of the current entry
 */
SQSH_NO_EXPORT uint32_t sqsh__directory_index_iterator_index(
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
SQSH_NO_EXPORT uint32_t sqsh__directory_index_iterator_start(
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
SQSH_NO_EXPORT uint32_t sqsh__directory_index_iterator_name_size(
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
SQSH_NO_EXPORT const char *sqsh__directory_index_iterator_name(
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
SQSH_NO_EXPORT int sqsh__directory_index_iterator_cleanup(
		struct SqshDirectoryIndexIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ITERATOR_PRIVATE_H */
