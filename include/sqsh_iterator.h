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
 * @file         sqsh_iterator.h
 */

#ifndef SQSH_ITERATOR_H
#define SQSH_ITERATOR_H

#include "sqsh_common.h"
#include "sqsh_context.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// iterator/directory_iterator.c

struct SqshDirectoryIterator;

/**
 * @memberof SqshDirectoryIterator
 * @brief Allocates and initializes a new directory iterator.
 *
 * @param[in]  inode      The inode to iterate through.
 * @param[out] err        Pointer to an int where the error code will be stored.
 *
 * @return The new iterator on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshDirectoryIterator *
sqsh_directory_iterator_new(struct SqshInodeContext *inode, int *err);

/**
 * @memberof SqshDirectoryIterator
 * @brief Advances the iterator to the next entry.
 *
 * @param[in,out] iterator The iterator to advance.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator);

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
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the size of the name of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the name on success, a negative value on error.
 */
int
sqsh_directory_iterator_name_size(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode reference of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode reference on success, a negative value on error.
 */
uint64_t
sqsh_directory_iterator_inode_ref(const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the inode type of the current entry.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The inode type on success, SqshInodeContextTypeInvalid on error.
 */
enum SqshInodeContextType sqsh_directory_iterator_inode_type(
		const struct SqshDirectoryIterator *iterator);

/**
 * @memberof SqshDirectoryIterator
 * @brief Loads the inode of the current entry.
 *
 * @param[in]  iterator The iterator to use.
 * @param[out] err  Pointer to an int where the error code will be stored.
 *
 * @return The loaded inode on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshInodeContext *sqsh_directory_iterator_inode_load(
		const struct SqshDirectoryIterator *iterator, int *err);

/**
 * @memberof SqshDirectoryIterator
 * @brief Retrieves the name of the current entry. Note that the name is not
 * null-terminated.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The name of the current entry.
 */
const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator);
/**
 * @memberof SqshDirectoryIterator
 * @brief creates a heap allocated copy of the name of the current entry.
 * The caller is responsible for freeing the memory.
 *
 * @param[in]  iterator    The iterator to use.
 * @param[out] name_buffer The buffer to hold the duplicated name.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_directory_iterator_name_dup(
		const struct SqshDirectoryIterator *iterator, char **name_buffer);
/**
 * @memberof SqshDirectoryIterator
 * @brief Frees the resources used by a directory iterator.
 *
 * @param[in] iterator The iterator to free.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_directory_iterator_free(struct SqshDirectoryIterator *iterator);

////////////////////////////////////////
// iterator/xattr_iterator.c

struct SqshXattrIterator;

/**
 * @memberof SqshXattrIterator
 * @brief Allocates and initializes a new xattr iterator.
 *
 * @param[in]  inode  The inode to iterate through xattrs.
 * @param[out] err    Pointer to an int where the error code will be stored.
 *
 * @return The new iterator on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshXattrIterator *
sqsh_xattr_iterator_new(const struct SqshInodeContext *inode, int *err);

/**
 * @memberof SqshXattrIterator
 * @brief Advances the iterator to the next xattr.
 *
 * @param[in,out] iterator The iterator to advance.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_xattr_iterator_next(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the type of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The type of the current xattr.
 */
uint16_t sqsh_xattr_iterator_type(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Checks if the current xattr is indirect.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return true if the xattr is indirect, false otherwise.
 */
bool sqsh_xattr_iterator_is_indirect(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the prefix of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The prefix of the current xattr.
 */
const char *sqsh_xattr_iterator_prefix(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the prefix of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the prefix of the current xattr.
 */
uint16_t sqsh_xattr_iterator_prefix_size(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the name of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The name of the current xattr.
 */
const char *sqsh_xattr_iterator_name(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the name of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the name of the current xattr.
 */
uint16_t sqsh_xattr_iterator_name_size(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Compares the full name of the current xattr with a given name.
 *
 * @param[in] iterator The iterator to use.
 * @param[in] name     The name to compare with.
 *
 * @return 0 if the names match, a negative value if the current xattr's name
 *         is less than the given name, a positive value if the current xattr's
 *         name is greater than the given name.
 */
int sqsh_xattr_iterator_fullname_cmp(
		struct SqshXattrIterator *iterator, const char *name);

/**
 * @memberof SqshXattrIterator
 * @brief creates a heap allocated copy of the full name of the current entry.
 * The caller is responsible for freeing the memory.
 *
 * @param[in]  iterator        The iterator to use.
 * @param[out] fullname_buffer The buffer to hold the duplicated full name.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_xattr_iterator_fullname_dup(
		struct SqshXattrIterator *iterator, char **fullname_buffer);

/**
 * @memberof SqshXattrIterator
 * @brief creates a heap allocated copy of the value of the current entry.
 * The caller is responsible for freeing the memory.
 *
 * @param[in]  iterator    The iterator to use.
 * @param[out] value_buffer The buffer to hold the duplicated value.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_xattr_iterator_value_dup(
		struct SqshXattrIterator *iterator, char **value_buffer);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the value of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The value of the current xattr.
 */
const char *sqsh_xattr_iterator_value(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the value of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the value of the current xattr.
 */
uint16_t sqsh_xattr_iterator_value_size(struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Frees the resources used by an xattr iterator.
 *
 * @param[in] iterator The iterator to free.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_xattr_iterator_free(struct SqshXattrIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_ITERATOR_H */
