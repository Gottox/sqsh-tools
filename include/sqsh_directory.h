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
 * @file         sqsh_directory.h
 */

#ifndef SQSH_DIRECTORY_H
#define SQSH_DIRECTORY_H

#include "sqsh_file.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * iterator/directory_iterator.c
 */

struct SqshDirectoryIterator;

/**
 * @memberof SqshDirectoryIterator
 * @brief Allocates and initializes a new directory iterator.
 *
 * @param[in]  file       file context of a directory to iterate over.
 * @param[out] err        Pointer to an int where the error code will be stored.
 *
 * @return The new iterator on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshDirectoryIterator *
sqsh_directory_iterator_new(const struct SqshFile *file, int *err);

/**
 * @memberof SqshDirectoryIterator
 * @brief Advances the iterator to the next entry.
 *
 * @param[in,out] iterator The iterator to advance.
 * @param[out]    err      Pointer to an int where the error code will be
 * stored.
 *
 * @retval true When the iterator has been advanced
 * @retval false When the iterator has reached the end of the directory or if an
 * error occured.
 */
SQSH_NO_UNUSED bool
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator, int *err);

/**
 * @memberof SqshDirectoryIterator
 * @brief Looks up an entry by name.
 *
 * @param[in,out] iterator The iterator to use.
 * @param[in]     name     The name of the entry to look up.
 * @param[in]     name_len The length of the name.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_directory_iterator_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len);

/**
 * @deprecated Since 1.3.0. Use sqsh_directory_iterator_name2() instead.
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the size of the name of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the name on success, a negative value on error.
 */
__attribute__((deprecated("Since 1.3.0. Use sqsh_directory_iterator_name2() "
						  "instead"))) uint16_t
sqsh_directory_iterator_name_size(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode number of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode number.
 */
uint32_t
sqsh_directory_iterator_inode(const struct SqshDirectoryIterator *iterator);

/**
 * @deprecated Since 1.2.0. Use sqsh_directory_iterator_inode() instead.
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode number of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode number.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_directory_iterator_inode() "
						  "instead"))) uint64_t
sqsh_directory_iterator_inode_number(
		const struct SqshDirectoryIterator *iterator);
/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode reference of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode reference.
 */
uint64_t
sqsh_directory_iterator_inode_ref(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode type of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode type on success, SQSH_INODE_TYPE_UNKNOWN on error.
 */
enum SqshFileType
sqsh_directory_iterator_file_type(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Loads the inode of the current entry.
 *
 * @param[in]  iterator The iterator to use.
 * @param[out] err  Pointer to an int where the error code will be stored.
 *
 * @return The loaded inode on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshFile *sqsh_directory_iterator_open_file(
		const struct SqshDirectoryIterator *iterator, int *err);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the name of the current entry.
 *
 * The returned pointer is allocated internally and only valid until the next
 * call of sqsh_directory_iterator_next(). It must not be freed. The returned
 * string is not 0 terminated.
 *
 * @param[in]  iterator  The iterator to use.
 * @param[out] len       Pointer to a size_t where the length of the name will
 * be stored.
 *
 * @return The name of the current entry.
 */
const char *sqsh_directory_iterator_name2(
		const struct SqshDirectoryIterator *iterator, size_t *len);

/**
 * @deprecated Since 1.3.0. Use sqsh_directory_iterator_name2() instead.
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the name of the current entry.
 *
 * The returned pointer is allocated internally and only valid until the next
 * call of sqsh_directory_iterator_next(). It must not be freed. The returned
 * string is not 0 terminated. Use sqsh_directory_iterator_name_size() to get
 * the size of the value.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The name of the current entry.
 */
__attribute__((deprecated("Since 1.3.0. Use sqsh_directory_iterator_inode() "
						  "instead"))) const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief creates a heap allocated copy of the name of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.
 *
 * @param[in]  iterator    The iterator to use.
 *
 * @return The name of the current entry.
 */
SQSH_NO_UNUSED char *
sqsh_directory_iterator_name_dup(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Frees the resources used by a directory iterator.
 *
 * @param[in] iterator The iterator to free.
 *
 * @return The file name of the current directory entry, NULL if the allocation
 * fails. The user is responsible for freeing the memory.
 */
int sqsh_directory_iterator_free(struct SqshDirectoryIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_DIRECTORY_H */
