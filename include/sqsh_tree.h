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

#include "sqsh_file.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

/***************************************
 * tree/path_resolver.c
 */

struct SqshPathResolver;

/**
 * @brief Creates a new SqshPathResolver object at the root inode.
 * @memberof SqshPathResolver
 *
 * @param[in]   archive  The archive to use
 * @param[out]  err      Pointer to an int where the error code will be stored.
 *
 * @return a new file reader.
 */
struct SqshPathResolver *
sqsh_path_resolver_new(struct SqshArchive *archive, int *err);

/**
 * @brief Moves the walker one level up
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_up(struct SqshPathResolver *walker);

/**
 * @memberof SqshPathResolver
 * @brief Moves the walker to the next entry int the current directory.
 *
 * @param[in,out]   walker  The walker to use
 * @param[out]      err     Pointer to an int where the error code will be
 * stored.
 *
 * @retval true if the walker was moved to the next entry.
 * @retval false if the walker has no more entries to move to or an error
 * occured.
 */
SQSH_NO_UNUSED bool
sqsh_path_resolver_next(struct SqshPathResolver *walker, int *err);

/**
 * @brief Returns the inode type of the current entry.
 * @memberof SqshPathResolver
 *
 * @param[in]   walker  The walker to use
 *
 * @return the inode type of the current entry.
 */
enum SqshFileType
sqsh_path_resolver_type(const struct SqshPathResolver *walker);

/**
 * @brief Returns the name of the current entry. This entry is not zero
 * terminated.
 * @memberof SqshPathResolver
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
const char *sqsh_path_resolver_name(const struct SqshPathResolver *walker);

/**
 * @memberof SqshPathResolver
 * @brief Returns the size of the name of the current entry.
 *
 * @param[in]   walker  The walker to use
 *
 * @return the size of the name of the current entry.
 */
uint16_t sqsh_path_resolver_name_size(const struct SqshPathResolver *walker);

/**
 * @memberof SqshPathResolver
 * @brief creates a heap allocated copy of the name of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
SQSH_NO_UNUSED char *
sqsh_path_resolver_name_dup(const struct SqshPathResolver *walker);

/**
 * @brief reverts the walker to the begining of the current directory.
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_revert(struct SqshPathResolver *walker);

/**
 * @brief Looks up an entry in the current directory.
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 * @param[in]       name    The name of the entry to look up.
 * @param[in]       name_size The size of the name.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_lookup(
		struct SqshPathResolver *walker, const char *name,
		const size_t name_size);

/**
 * @brief Lets the walker enter the current entry.
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_down(struct SqshPathResolver *walker);

/**
 * @brief Moves the walker to the root directory.
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_to_root(struct SqshPathResolver *walker);

/**
 * @brief Returns the inode of the current entry.
 * @memberof SqshPathResolver
 *
 * @param[in,out]   walker  The walker to use
 * @param[out]      err     Pointer to an int where the error code will be
 *
 * @return the inode of the current entry.
 */
SQSH_NO_UNUSED struct SqshFile *
sqsh_path_resolver_open_file(const struct SqshPathResolver *walker, int *err);

/**
 * @memberof SqshPathResolver
 * @brief Resolve a path with the tree walker.
 *
 * This function will resolve the given path with the tree walker. The base is
 * the current directory.
 *
 * @param[in,out]   walker           The walker to use
 * @param[in]       path             The path to resolve.
 * @param[in]       follow_symlinks  Whether to follow symlinks.
 *
 * @return the inode of the current entry.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_resolve(
		struct SqshPathResolver *walker, const char *path,
		bool follow_symlinks);

/**
 * @brief Cleans up resources used by a SqshPathResolver struct.
 * @memberof SqshPathResolver
 *
 * @param[in,out] reader The file reader struct to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_free(struct SqshPathResolver *reader);

/***************************************
 * tree/walker.c
 */

struct SqshTreeWalker;

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_new() instead.
 * @brief Creates a new SqshTreeWalker object at the root inode.
 * @memberof SqshTreeWalker
 *
 * @param[in]   archive  The archive to use
 * @param[out]  err      Pointer to an int where the error code will be stored.
 *
 * @return a new file reader.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_new() "
						  "instead."))) struct SqshTreeWalker *
sqsh_tree_walker_new(struct SqshArchive *archive, int *err);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_up() instead.
 * @brief Moves the walker one level up
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_up() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_up(struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_next() instead.
 * @memberof SqshTreeWalker
 * @brief Moves the walker to the next entry int the current directory.
 *
 * This function was deprecated to align the API with other iterator APIs. The
 * `sqsh_tree_walker_next2()` uses the same signature as the other iterator.
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_next() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_next(struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_type() instead.
 * @brief Returns the inode type of the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the inode type of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_type() "
						  "instead."))) enum SqshFileType
sqsh_tree_walker_type(const struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_name() instead.
 * @brief Returns the name of the current entry. This entry is not zero
 * terminated.
 * @memberof SqshTreeWalker
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_name() "
						  "instead."))) const char *
sqsh_tree_walker_name(const struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_name_size() instead.
 * @memberof SqshTreeWalker
 * @brief Returns the size of the name of the current entry.
 *
 * @param[in]   walker  The walker to use
 *
 * @return the size of the name of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_name_size() "
						  "instead."))) uint16_t
sqsh_tree_walker_name_size(const struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_name_dup() instead.
 * @memberof SqshTreeWalker
 * @brief creates a heap allocated copy of the name of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.
 *
 * @param[in]   walker  The walker to use
 *
 * @return the name of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_name_dup() "
						  "instead."))) SQSH_NO_UNUSED char *
sqsh_tree_walker_name_dup(const struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_revert() instead.
 * @brief reverts the walker to the begining of the current directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_revert() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_revert(struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_lookup() instead.
 * @brief Looks up an entry in the current directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 * @param[in]       name    The name of the entry to look up.
 * @param[in]       name_size The size of the name.
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_lookup() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_lookup(
		struct SqshTreeWalker *walker, const char *name,
		const size_t name_size);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_down() instead.
 * @brief Lets the walker enter the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_down() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_down(struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_to_root() instead.
 * @brief Moves the walker to the root directory.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_to_root() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_to_root(struct SqshTreeWalker *walker);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_open_file() instead.
 * @brief Returns the inode of the current entry.
 * @memberof SqshTreeWalker
 *
 * @param[in,out]   walker  The walker to use
 * @param[out]      err     Pointer to an int where the error code will be
 *
 * @return the inode of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_open_file() "
						  "instead."))) SQSH_NO_UNUSED struct SqshFile *
sqsh_tree_walker_open_file(const struct SqshTreeWalker *walker, int *err);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_open_file() instead.
 * @memberof SqshTreeWalker
 * @brief Resolve a path with the tree walker.
 *
 * This function will resolve the given path with the tree walker. The base is
 * the current directory.
 *
 * @param[in,out]   walker           The walker to use
 * @param[in]       path             The path to resolve.
 * @param[in]       follow_symlinks  Whether to follow symlinks.
 *
 * @return the inode of the current entry.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_resolve() "
						  "instead."))) SQSH_NO_UNUSED int
sqsh_tree_walker_resolve(
		struct SqshTreeWalker *walker, const char *path, bool follow_symlinks);

/**
 * @deprecated Since 1.2.0. Use sqsh_path_resolver_free() instead.
 * @brief Cleans up resources used by a SqshTreeWalker struct.
 * @memberof SqshTreeWalker
 *
 * @param[in,out] reader The file reader struct to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
__attribute__((deprecated("Since 1.2.0. Use sqsh_path_resolver_free() "
						  "instead."))) int
sqsh_tree_walker_free(struct SqshTreeWalker *reader);

/***************************************
 * tree/traversal.c
 */

enum SqshTreeTraversalState {
	SQSH_TREE_TRAVERSAL_STATE_INIT,
	SQSH_TREE_TRAVERSAL_STATE_FILE,
	SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN,
	SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END,
};

struct SqshTreeTraversal;

/**
 * @deprecated Since 1.4.0. Use sqsh_tree_traversal_new2() instead.
 * @brief Creates a new SqshTreeTraversal object at the root inode.
 * @memberof SqshTreeTraversal
 *
 * @param[in]   file     the base inode to start from.
 * @param[out]  err      Pointer to an int where the error code will be
 * stored.
 *
 * @return a new file reader.
 */
__attribute__((deprecated("Since 1.4.0. Use sqsh_tree_traversal_new2() "
						  "instead."))) struct SqshTreeTraversal *
sqsh_tree_traversal_new(const struct SqshFile *file, int *err);

/**
 * @brief Creates a new SqshTreeTraversal object at the root inode.
 * @memberof SqshTreeTraversal
 *
 * @param[in]   file     the base inode to start from.
 * @param[in]   max_depth the maximum depth to traverse.
 * @param[out]  err      Pointer to an int where the error code will be
 * stored.
 *
 * @return a new file reader.
 */
struct SqshTreeTraversal *sqsh_tree_traversal_new2(
		const struct SqshFile *file, size_t max_depth, int *err);

/**
 * @memberof SqshTreeTraversal
 * @brief Moves the traversal to the next entry int the current directory.
 *
 * @param[in,out]   traversal  The traversal to use
 * @param[out]      err Pointer to an int where the error code will be stored.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED bool
sqsh_tree_traversal_next(struct SqshTreeTraversal *traversal, int *err);

/**
 * @brief Returns the inode type of the current entry.
 * @memberof SqshTreeTraversal
 *
 * @param[in]   traversal  The traversal to use
 *
 * @return the inode type of the current entry.
 */
enum SqshFileType
sqsh_tree_traversal_type(const struct SqshTreeTraversal *traversal);

/**
 * @memberof SqshTreeTraversal
 * @brief returns the state of the traversal.
 *
 * @param[in,out]   traversal  The traversal to use
 *
 * @return 0 on success, less than 0 on error.
 */
enum SqshTreeTraversalState
sqsh_tree_traversal_state(const struct SqshTreeTraversal *traversal);

/**
 * @brief Returns the name of the current entry. This entry is not zero
 * terminated.
 * @memberof SqshTreeTraversal
 *
 * @param[in]   traversal  The traversal to use
 * @param[out]  len        Pointer to a size_t where the length of the name will
 * be stored.
 *
 * @return the name of the current entry.
 */
const char *sqsh_tree_traversal_name(
		const struct SqshTreeTraversal *traversal, size_t *len);

/**
 * @brief Returns the name of the current entry. This entry is not zero
 * terminated.
 * @memberof SqshTreeTraversal
 *
 * @param[in]   traversal  The traversal to use
 *
 * @return the name of the current entry.
 */
char *sqsh_tree_traversal_path_dup(const struct SqshTreeTraversal *traversal);

/**
 * @memberof SqshTreeTraversal
 * @brief creates a heap allocated copy of the name of the current entry.
 *
 * The caller is responsible for calling free() on the returned pointer.
 *
 * The returned string is 0 terminated.
 *
 * @param[in]   traversal  The traversal to use
 *
 * @return the name of the current entry.
 */
SQSH_NO_UNUSED char *
sqsh_tree_traversal_name_dup(const struct SqshTreeTraversal *traversal);

/**
 * @brief Returns the path segment at a given index.
 * @memberof SqshTreeTraversal
 *
 * @param[in,out]   traversal  The traversal to use
 *
 * @return the inode of the current entry.
 */
size_t sqsh_tree_traversal_depth(const struct SqshTreeTraversal *traversal);

/**
 * @brief Returns the length of the path segment at a given index.
 * @memberof SqshTreeTraversal
 *
 * @param[in,out]   traversal  The traversal to use
 * @param[out]      len        Pointer to a size_t where the length of the name
 * will be stored.
 * @param[in]       index      The index of the path segment.
 *
 * @return the inode of the current entry.
 */
const char *sqsh_tree_traversal_path_segment(
		const struct SqshTreeTraversal *traversal, size_t *len,
		sqsh_index_t index);

/**
 * @brief Gets the underlying directory iterator pointing to the current entry.
 *
 * @param[in]   traversal  The traversal to use
 *
 * @return the inode of the current entry.
 */
const struct SqshDirectoryIterator *
sqsh_tree_traversal_iterator(const struct SqshTreeTraversal *traversal);

/**
 * @brief Returns the inode of the current entry.
 * @memberof SqshTreeTraversal
 *
 * @param[in,out]   traversal  The traversal to use
 * @param[out]      err     Pointer to an int where the error code will be
 *
 * @return the inode of the current entry.
 */
SQSH_NO_UNUSED struct SqshFile *sqsh_tree_traversal_open_file(
		const struct SqshTreeTraversal *traversal, int *err);

/**
 * @memberof SqshTreeTraversal
 *
 * @param[in,out] traversal The file traversal struct to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_tree_traversal_free(struct SqshTreeTraversal *traversal);

#ifdef __cplusplus
}
#endif
#endif // SQSH_TREE_H
