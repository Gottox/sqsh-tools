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
 * @file         sqsh_archive_private.h
 */

#ifndef SQSH_ARCHIVE_PRIVATE_H
#define SQSH_ARCHIVE_PRIVATE_H

#include "sqsh_archive.h"
#include "sqsh_compression_private.h"
#include "sqsh_error.h"
#include "sqsh_xattr_private.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// archive/superblock_context.c

struct SqshSuperblockContext {
	/**
	 * @privatesection
	 */
	struct SqshMapCursor cursor;
};

/**
 * @internal
 * @memberof SqshSuperblockContext
 * @brief Initializes a superblock context.
 *
 * @param[out] context The context to initialize.
 * @param[in]  mapper  The mapper to use for the superblock.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__superblock_init(
		struct SqshSuperblockContext *context, struct SqshMapManager *mapper);

/**
 * @internal
 * @memberof SqshSuperblockContext
 * @brief Cleans up a superblock context.
 *
 * @param[in] superblock The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__superblock_cleanup(struct SqshSuperblockContext *superblock);

////////////////////////////////////////
// archive/compression_options_context.c

/**
 * @brief The compression options context is used to store the
 * compression options for a specific compression algorithm.
 */
struct SqshCompressionOptionsContext {
	/**
	 * @privatesection
	 */
	uint16_t compression_id;
	struct SqshMetablockIterator metablock;
};

/**
 * @internal
 * @memberof SqshCompressionOptionsContext
 * @brief Initialize the compression options context.
 * @param context the compression options context
 * @param sqsh the Sqsh struct
 */
SQSH_NO_UNUSED int sqsh__compression_options_init(
		struct SqshCompressionOptionsContext *context,
		struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshCompressionOptionsContext
 * @brief Frees the resources used by the compression options context.
 * @param context the compression options context
 */
int sqsh__compression_options_cleanup(
		struct SqshCompressionOptionsContext *context);

////////////////////////////////////////
// archive/archive.c

/**
 * @brief The Sqsh struct contains all information about the current
 * sqsh session.
 */
struct SqshArchive {
	/**
	 * @privatesection
	 */
	struct SqshMapManager map_manager;
	struct SqshCompression data_compression;
	struct SqshCompression metablock_compression;
	struct SqshSuperblockContext superblock;
	struct SqshTable id_table;
	struct SqshTable export_table;
	struct SqshXattrTable xattr_table;
	struct SqshFragmentTable fragment_table;
	uint8_t initialized;
	struct SqshConfig config;
};

/**
 * @internal
 * @brief sqsh__init initializes the Sqsh structure.
 * @memberof SqshArchive
 *
 * @param sqsh the Sqsh structure to initialize.
 * @param source the source to retrieve the archive from
 * @param config the configuration for the Sqsh session.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__archive_init(
		struct SqshArchive *sqsh, const void *source,
		const struct SqshConfig *config);

/**
 * @internal
 * @brief sqsh__cleanup frees all resources allocated by the Sqsh structure and
 * cleans up the structure.
 * @memberof SqshArchive
 *
 * @param sqsh the Sqsh structure to cleanup.
 * @return 0 on success, less than 0 on error.
 */
int sqsh__archive_cleanup(struct SqshArchive *sqsh);

#ifdef __cplusplus
}
#endif
#endif // SQSH_ARCHIVE_PRIVATE_H