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
 * @file         sqsh_context.h
 */

#ifndef SQSH_CONTEXT_H
#define SQSH_CONTEXT_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// context/path_resolver_context.c

struct SqshPathResolverContext;

/**
 * @memberof SqshPathResolverContext
 * @brief initializes a path resolver context in heap
 *
 * @param[in]  sqsh The sqsh context.
 * @param[out] err  Pointer to an int where the error code will be stored.
 *
 * @return The Initialized path resolver context
 */
struct SqshPathResolverContext *
sqsh_path_resolver_new(struct SqshArchive *sqsh, int *err);

/**
 * @memberof SqshPathResolverContext
 * @brief Initialize the inode context from a path.
 *
 * @param[in] context The path resolver context.
 * @param[in] path The path the file or directory.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return an inode context on success, NULL on error
 */
SQSH_NO_UNUSED struct SqshInodeContext *sqsh_path_resolver_resolve(
		struct SqshPathResolverContext *context, const char *path, int *err);

/**
 * @memberof SqshPathResolverContext
 * @brief cleans up a path resolver context and frees the memory.
 *
 * @param[in] context The path resolver context.
 *
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_free(struct SqshPathResolverContext *context);

////////////////////////////////////////
// context/trailing_context.c

struct SqshTrailingContext;

/**
 * @memberof SqshTrailingContext
 * @brief Retrieves the size of the trailing data in a context.
 *
 * @param[in] context The context to retrieve the size from.
 *
 * @return The size of the trailing data in the context.
 */
size_t sqsh_trailing_size(const struct SqshTrailingContext *context);

/**
 * @memberof SqshTrailingContext
 * @brief Retrieves the trailing data in a context.
 *
 * @param[in] context The context to retrieve the data from.
 *
 * @return The trailing data in the context.
 */
const uint8_t *sqsh_trailing_data(const struct SqshTrailingContext *context);

#ifdef __cplusplus
}
#endif
#endif // SQSH_CONTEXT_H
