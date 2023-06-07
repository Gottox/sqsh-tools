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
 * @file         sqsh_tree.h
 */

#ifndef SQSH_TREE_H
#define SQSH_TREE_H

#include "sqsh_common.h"
#include "sqsh_inode.h"
#include "sqsh_table_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * tree/walker.c
 */

struct SqshTreeWalker;

/**
 * @brief Initializes a SqshTreeWalker struct.
 * @memberof SqshTreeWalker
 *
 * @param[in]   archive  The archive to use
 * @param[out]  err      Pointer to an int where the error code will be stored.
 *
 * @return a new file reader.
 */
struct SqshTreeWalker *
sqsh_tree_walker_new(struct SqshArchive *archive, int *err);

/**
 * @brief Moves the walker one level up
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_up(struct SqshTreeWalker *walker);

/**
 * @brief Moves the walker to the next entry int the current directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_next(struct SqshTreeWalker *walker);

/**
 * @brief Returns the inode type of the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the inode type of the current entry.
 */
enum SqshInodeType sqsh_tree_walker_type(const struct SqshTreeWalker *walker);

/**
 * @brief Returns the name of the current entry. This entry is not zero
 * terminated.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
const char *sqsh_tree_walker_name(const struct SqshTreeWalker *walker);

/**
 * @brief Returns the size of the name of the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the size of the name of the current entry.
 */
uint16_t sqsh_tree_walker_name_size(const struct SqshTreeWalker *walker);

/**
 * @brief Returns the name of the current entry. This entry is zero terminated.
 * The user is responsible for freeing the memory.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
SQSH_NO_UNUSED char *
sqsh_tree_walker_name_dup(const struct SqshTreeWalker *walker);

/**
 * @brief reverts the walker to the begining of the current directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_revert(struct SqshTreeWalker *walker);

/**
 * @brief Looks up an entry in the current directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 * @param[in]       name    The name of the entry to look up.
 * @param[in]       name_size The size of the name.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_lookup(
		struct SqshTreeWalker *walker, const char *name,
		const size_t name_size);

/**
 * @brief Lets the walker enter the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_down(struct SqshTreeWalker *walker);

/**
 * @brief Moves the walker to the root directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_to_root(struct SqshTreeWalker *walker);

/**
 * @brief Returns the inode of the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 * @param[out]      err     Pointer to an int where the error code will be
 *
 * @return the inode of the current entry.
 */
SQSH_NO_UNUSED struct SqshInode *
sqsh_tree_walker_inode_load(const struct SqshTreeWalker *walker, int *err);

/**
 * @brief Resolve a path with the tree walker.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker           The walker to use
 * @param[in]       path             The path to resolve.
 * @param[in]       follow_symlinks  Whether to follow symlinks.
 *
 * @return the inode of the current entry.
 */
SQSH_NO_UNUSED int sqsh_tree_walker_resolve(
		struct SqshTreeWalker *walker, const char *path, bool follow_symlinks);

/**
 * @brief Cleans up resources used by a SqshTreeWalker struct.
 * @memberof SqshTreeWalker
 *
 * @param[in,out] reader The file reader struct to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_tree_walker_free(struct SqshTreeWalker *reader);

#ifdef __cplusplus
}
#endif
#endif // SQSH_TREE_H
