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
// iterator/directory_index_iterator.c

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

////////////////////////////////////////
// iterator/metablock_iterator.c

struct SqshMetablockIterator {
	/**
	 * @privatesection
	 */
	struct SqshMapCursor cursor;
	bool is_compressed;
	struct SqshCompression *compression;
	uint16_t size;
};

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Initializes an iterator for reading metablocks.
 *
 * @param[out] iterator The iterator to initialize.
 * @param[in] sqsh The Squash instance the metablocks belong to.
 * @param[in] start_address The address of the first metablock to read.
 * @param[in] upper_limit The maximum address the iterator is allowed to read.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_init(
		struct SqshMetablockIterator *iterator, struct Sqsh *sqsh,
		uint64_t start_address, uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief skip several metablocks. sqsh__metablock_iterator_skip(i, 1) is
 * equivalent to sqsh__metablock_iterator_next(i).
 *
 * @param[in,out] iterator The iterator to advance.
 * @param[in] amount The number of metablocks to skip.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_skip(
		struct SqshMetablockIterator *iterator, uint64_t amount);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Moves the iterator to the next metablock.
 *
 * @param[in,out] iterator The iterator to advance.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_next(struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns a pointer to the data of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return A pointer to the data of the current metablock.
 */
const uint8_t *
sqsh__metablock_iterator_data(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns whether the current metablock is compressed.
 *
 * @param[in] iterator The iterator.
 * @return Whether the current metablock is compressed.
 */
bool sqsh__metablock_iterator_is_compressed(
		const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns the size of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return The size of the current metablock.
 */
size_t
sqsh__metablock_iterator_size(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Appends the data of the current metablock to a buffer.
 *
 * @param[in] iterator The iterator.
 * @param[out] buffer The buffer to append the data to.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_append_to_buffer(
		const struct SqshMetablockIterator *iterator,
		struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Cleans up an iterator for reading metablocks.
 *
 * @param[in,out] iterator The iterator to clean up.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_cleanup(struct SqshMetablockIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_ITERATOR_PRIVATE_H */
