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
 * @file         sqsh_xattr.h
 */

#ifndef SQSH_XATTR_H
#define SQSH_XATTR_H

#include "sqsh_common.h"
#include "sqsh_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshFile;

/***************************************
 * xattr/xattr_iterator.c
 */

struct SqshXattrIterator;

/**
 * @memberof SqshXattrIterator
 * @brief Allocates and initializes a new xattr iterator.
 *
 * @param[in]  file   The file context to iterate through xattrs.
 * @param[out] err    Pointer to an int where the error code will be stored.
 *
 * @return The new iterator on success, NULL on error.
 */
SQSH_NO_UNUSED struct SqshXattrIterator *
sqsh_xattr_iterator_new(const struct SqshFile *file, int *err);

/**
 * @memberof SqshXattrIterator
 * @brief Advances the iterator to the next xattr.
 *
 * @param[in,out] iterator The iterator to advance.
 * @param[out]    err      Pointer to an int where the error code will be
 * stored.
 *
 * @retval true  When the iterator was advanced.
 * @retval false When the end of the xattrs list was reached or an error
 * occured.
 */
SQSH_NO_UNUSED bool
sqsh_xattr_iterator_next(struct SqshXattrIterator *iterator, int *err);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the type of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The type of the current xattr.
 */
uint16_t sqsh_xattr_iterator_type(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Checks if the current xattr is indirect.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return true if the xattr is indirect, false otherwise.
 */
bool sqsh_xattr_iterator_is_indirect(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the prefix of the current xattr.
 *
 * There are three possible prefixes that can be returned:
 *
 * - "user."
 * - "trusted."
 * - "security."
 *
 * The returned pointer is staticly allocated and must not be freed.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The null terminated prefix of the current xattr. The returned pointer
 * is staticly allocated and must not be freed.
 */
const char *
sqsh_xattr_iterator_prefix(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the prefix of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the prefix of the current xattr.
 */
uint16_t
sqsh_xattr_iterator_prefix_size(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the name of the current xattr excluding the prefix.
 *
 * The returned pointer is allocated internally and only valid until the next
 * call to sqsh_xattr_iterator_next(). It must not be freed. The string is not 0
 * terminated. Use sqsh_xattr_iterator_name_size() to get the size of the name.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The name of the current xattr.
 */
const char *sqsh_xattr_iterator_name(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the name of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the name of the current xattr.
 */
uint16_t
sqsh_xattr_iterator_name_size(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Looks up an xattr by name.
 *
 * @param[in] iterator The iterator to use.
 * @param[in] name     The name to lookup
 *
 * @return 0 if the xattr was found, a negative value otherwise.
 */
SQSH_NO_UNUSED int sqsh_xattr_iterator_lookup(
		struct SqshXattrIterator *iterator, const char *name);

/**
 * @memberof SqshXattrIterator
 * @brief Compares the full name of the current xattr with a given 0-terminated
 * name.
 *
 * @param[in] iterator The iterator to use.
 * @param[in] name     The name to compare with.
 *
 * @return 0 if the names match, a negative value if the current xattr's name
 *         is less than the given name, a positive value if the current xattr's
 *         name is greater than the given name.
 */
SQSH_NO_UNUSED int sqsh_xattr_iterator_fullname_cmp(
		const struct SqshXattrIterator *iterator, const char *name);

/**
 * @memberof SqshXattrIterator
 * @brief creates a heap allocated copy of the full name of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.
 *
 * @param[in]  iterator        The iterator to use.
 *
 * @return The full name of the current xattr on success, NULL if the allocation
 * fails.
 */
SQSH_NO_UNUSED char *
sqsh_xattr_iterator_fullname_dup(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief creates a heap allocated copy of the value of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.

 *
 * @param[in]  iterator    The iterator to use.
 *
 * @return The value of the current xattr on success, NULL if the allocation
 * fails.
 */
SQSH_NO_UNUSED char *
sqsh_xattr_iterator_value_dup(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the value of the current xattr.
 *
 * The returned pointer is allocated internally and only valid until the next
 * call to sqsh_xattr_iterator_next(). It must not be freed. The string is not 0
 * terminated. Use sqsh_xattr_iterator_value_size() to get the size of the
 * value.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The value of the current xattr. The returned pointer is allocated
 * internally and only valid until the next call to sqsh_xattr_iterator_next().
 * It must not be freed.
 */
const char *sqsh_xattr_iterator_value(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the value of the current xattr.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the value of the current xattr.
 */
uint32_t
sqsh_xattr_iterator_value_size2(const struct SqshXattrIterator *iterator);

/**
 * @memberof SqshXattrIterator
 * @brief Retrieves the size of the value of the current xattr.
 * @deprecated Since 1.3.0. Use sqsh_xattr_iterator_value_size2() instead.
 *
 * @param[in] iterator The iterator to use.
 *
 * @return The size of the value of the current xattr.
 */
__attribute__((deprecated(
		"Since 1.3.0. Use sqsh_xattr_iterator_value_size2() instead.")))
uint16_t
sqsh_xattr_iterator_value_size(const struct SqshXattrIterator *iterator);

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
#endif /* SQSH_XATTR_H */
