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

#ifndef SQSH_CONTEXT_PRIVATE_H
#define SQSH_CONTEXT_PRIVATE_H

#include "sqsh_context.h"
#include "sqsh_metablock_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// context/path_resolver.c

struct SqshPathResolver {
	/**
	 * @privatesection
	 */
	struct SqshArchive *sqsh;
};

/**
 * @internal
 * @memberof SqshPathResolver
 * @brief initializes a path resolver context.
 *
 * @param[out] resolver The path resolver context.
 * @param[in] sqsh The sqsh context.
 *
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__path_resolver_init(
		struct SqshPathResolver *resolver, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshPathResolver
 * @brief cleans up a path resolver context.
 *
 * @param[in] resolver The path resolver context.
 *
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_cleanup(struct SqshPathResolver *resolver);

////////////////////////////////////////
// context/trailing_context.c

struct SqshTrailingContext {
	/**
	 * @privatesection
	 */
	struct SqshMapReader cursor;
};

/**
 * @internal
 * @memberof SqshTrailingContext
 * @brief Initializes a trailing context.
 *
 * @param[out] context The context to initialize.
 * @param[in]  sqsh The Sqsh instance to use for the context.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__trailing_init(
		struct SqshTrailingContext *context, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshTrailingContext
 * @brief Cleans up a trailing context.
 *
 * @param[in] context The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__trailing_cleanup(struct SqshTrailingContext *context);

#ifdef __cplusplus
}
#endif
#endif // SQSH_CONTEXT_PRIVATE_H
