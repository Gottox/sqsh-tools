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

#ifndef SQSH_TREE_PRIVATE_H
#define SQSH_TREE_PRIVATE_H

#include "sqsh_directory_private.h"
#include "sqsh_inode_private.h"
#include "sqsh_tree.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// tree/walker.c

/**
 * @brief A walker over the contents of a file.
 */
struct SqshTreeWalker {
	/**
	 * @privatesection
	 */
	uint64_t root_inode_ref;
	uint64_t current_inode_ref;
	struct SqshArchive *archive;
	struct SqshInodeCache *inode_cache;
	struct SqshInode directory;
	struct SqshDirectoryIterator iterator;
	bool begin_iterator;
};

/**
 * @internal
 * @memberof SqshTreeWalker
 * @brief Initializes a SqshTreeWalker struct.
 *
 * @param[out] walker   The file walker to initialize.
 * @param[in]  archive  The archive to use
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__tree_walker_init(
		struct SqshTreeWalker *walker, struct SqshArchive *archive);

/**
 * @internal
 * @memberof SqshTreeWalker
 * @brief Frees the resources used by the file walker.
 *
 * @param walker The file walker to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh__tree_walker_cleanup(struct SqshTreeWalker *walker);

#ifdef __cplusplus
}
#endif
#endif // SQSH_TREE_PRIVATE_H
