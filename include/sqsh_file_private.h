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
 * @file         sqsh_file.h
 */

#ifndef SQSH_FILE_PRIVATE_H
#define SQSH_FILE_PRIVATE_H

#include "sqsh_file.h"
#include "sqsh_mapper_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// file/fragment_table.c

struct SqshFragmentTable {
	/**
	 * @privatesection
	 */
	const struct SqshSuperblockContext *superblock;
	struct SqshTable table;
	struct SqshMapManager *map_manager;
	struct SqshCompressionManager compression_manager;
};

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Initializes a fragment table with a SQSH context.
 *
 * @param[out] context The fragment table to initialize.
 * @param[in]  sqsh The SQSH context to use for the fragment table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__fragment_table_init(
		struct SqshFragmentTable *context, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Cleans up a fragment table.
 *
 * @param[in] context The fragment table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__fragment_table_cleanup(struct SqshFragmentTable *context);

////////////////////////////////////////
// context/file_reader.c

struct SqshFileReader {
	/**
	 * @privatesection
	 */
	const struct SqshCompression *compression;
	const struct SqshSuperblockContext *superblock;
	struct SqshMapManager *map_manager;
	sqsh_index_t datablock_index;
	sqsh_index_t datablock_offset;
	struct SqshBuffer buffer;
	struct SqshMapReader map_reader;
	const uint8_t *data;
	size_t data_size;
	const struct SqshInodeContext *inode;
};

/**
 * @internal
 * @brief Initializes a SqshFileContext struct.
 * @memberof SqshFileContext
 *
 * @param[out] reader The file reader to initialize.
 * @param[in] inode    The inode context to retrieve the file contents from.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__file_reader_init(
		struct SqshFileReader *reader, const struct SqshInodeContext *inode);

/**
 * @internal
 * @brief Frees the resources used by the file reader.
 *
 * @memberof SqshFileContext
 *
 * @param reader The file reader to clean up.
 */
int sqsh__file_reader_cleanup(struct SqshFileReader *reader);

////////////////////////////////////////
// context/file_context.c

/**
 * @brief The SqshFileContext struct
 *
 * This struct is used to assemble file contents.
 */
struct SqshFileContext {
	/**
	 * @privatesection
	 */
	struct SqshMapManager *map_manager;
	struct SqshFragmentTable *fragment_table;
	const struct SqshInodeContext *inode;
	struct SqshBuffer buffer;
	const struct SqshCompression *compression;
	uint64_t seek_pos;
	uint32_t block_size;
};

/**
 * @internal
 * @brief Initializes a SqshFileContext struct.
 * @memberof SqshFileContext
 *
 * @param[out] context The file context to initialize.
 * @param[in] inode    The inode context to retrieve the file contents from.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__file_init(
		struct SqshFileContext *context, const struct SqshInodeContext *inode);

/**
 * @internal
 * @brief Frees the resources used by the file context.
 *
 * @memberof SqshFileContext
 *
 * @param context The file context to clean up.
 */
int sqsh__file_cleanup(struct SqshFileContext *context);

#ifdef __cplusplus
}
#endif
#endif // SQSH_FILE_PRIVATE_H
