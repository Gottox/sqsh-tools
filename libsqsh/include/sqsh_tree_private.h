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
 * @file         sqsh_tree_private.h
 */

#ifndef SQSH_TREE_PRIVATE_H
#define SQSH_TREE_PRIVATE_H

#include "sqsh_directory_private.h"
#include <sqsh_tree.h>

#include <cextras/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

/***************************************
 * tree/path_resolver.c
 */

/**
 * @brief Resolves paths
 */
struct SqshPathResolver {
	/**
	 * @privatesection
	 */
	size_t max_symlink_depth;
	size_t current_symlink_depth;
	struct SqshFile cwd;
	struct SqshDirectoryIterator iterator;
	struct SqshArchive *archive;
	struct SqshInodeMap *inode_map;
	uint64_t current_inode_ref;
	uint64_t root_inode_ref;
};

SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__path_resolver_init(
		struct SqshPathResolver *resolver, struct SqshArchive *archive);

SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__path_resolver_to_ref(
		struct SqshPathResolver *resolver, uint64_t inode_ref);

SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__path_resolver_to_inode(
		struct SqshPathResolver *resolver, uint32_t inode_number);

SQSH_NO_EXPORT enum SqshFileType
sqsh__path_resolver_type(const struct SqshPathResolver *resolver);

SQSH_NO_EXPORT SQSH_NO_UNUSED int
sqsh__path_resolver_follow_symlink(struct SqshPathResolver *resolver);

SQSH_NO_EXPORT SQSH_NO_UNUSED int
sqsh__path_resolver_follow_all_symlinks(struct SqshPathResolver *resolver);

SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__path_resolver_resolve_nt(
		struct SqshPathResolver *resolver, const char *path, size_t path_len,
		bool follow_symlinks);

SQSH_NO_EXPORT int
sqsh__path_resolver_cleanup(struct SqshPathResolver *resolver);

/***************************************
 * tree/traversal.c
 */

/**
 * @brief A walker over the contents of a file.
 */
struct SqshTreeTraversalStackElement {
	struct SqshDirectoryIterator iterator;
	struct SqshFile file;
	struct SqshTreeTraversalStackElement *next;
};

/**
 * @brief A walker over the contents of a file.
 */
struct SqshTreeTraversal {
	/**
	 * @privatesection
	 */

	struct CxPreallocPool stack_pool;
	struct SqshTreeTraversalStackElement *stack;
	size_t stack_size;

	size_t depth;
	size_t max_depth;

	const struct SqshFile *base_file;
	struct SqshDirectoryIterator base_iterator;

	const struct SqshFile *current_file;
	struct SqshDirectoryIterator *current_iterator;
	enum SqshTreeTraversalState state;
};

/**
 * @internal
 * @memberof SqshTreeTraversal
 * @brief Initializes a SqshTreeTraversal struct.
 *
 * @param[out] traversal  The file traversal to initialize.
 * @param[in]  file       the file to start traversal at
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__tree_traversal_init(
		struct SqshTreeTraversal *traversal, const struct SqshFile *file);

/**
 * @internal
 * @memberof SqshTreeTraversal
 * @brief Frees the resources used by the file traversal.
 *
 * @param traversal The file traversal to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT int
sqsh__tree_traversal_cleanup(struct SqshTreeTraversal *traversal);

/***************************************
 * tree/walker.c
 */

/**
 * @brief A walker over the contents of a file.
 * @deprecated Since 1.2.0. Use struct SqshPathResolver instead.
 */
struct SqshTreeWalker {
	/**
	 * @privatesection
	 */
	struct SqshPathResolver inner;
} __attribute__((deprecated("Since 1.2.0. Use SqshPathResolver instead.")));

#ifdef __cplusplus
}
#endif
#endif /* SQSH_TREE_PRIVATE_H */
