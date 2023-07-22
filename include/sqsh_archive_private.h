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

#include "sqsh_error.h"
#include "sqsh_file_private.h"
#include "sqsh_thread_private.h"
#include "sqsh_xattr_private.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * archive/trailing_context.c
 */

/**
 * @brief The trailing context is used to access the trailing data
 * of the archive.
 */
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

/***************************************
 * archive/inode_map.c
 */

/**
 * @brief The inode map context is used to cache inodes numbers and their
 * corresponding inode references.
 */
struct SqshInodeMap {
	/**
	 * @privatesection
	 */
	void *inode_refs; /* atomic_uint_fast64_t */
	size_t inode_count;
	struct SqshExportTable *export_table;
};

/**
 * @internal
 * @memberof SqshInodeMap
 * @brief Initializes an inode map context.
 *
 * @param[out] map The context to initialize.
 * @param[in]  archive The archive to use for the context.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh__inode_map_init(struct SqshInodeMap *map, struct SqshArchive *archive);

/**
 * @internal
 * @memberof SqshInodeMap
 * @brief Cleans up an inode map context.
 *
 * @param[in] map The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__inode_map_cleanup(struct SqshInodeMap *map);

/***************************************
 * archive/superblock.c
 */

/**
 * @brief The superblock context is used to access the superblock of
 * the archive.
 */
struct SqshSuperblock {
	/**
	 * @privatesection
	 */
	struct SqshMapReader cursor;
};

/**
 * @internal
 * @memberof SqshSuperblock
 * @brief Initializes a superblock context.
 *
 * @param[out] superblock The context to initialize.
 * @param[in]  mapper     The mapper to use for the superblock.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__superblock_init(
		struct SqshSuperblock *superblock, struct SqshMapManager *mapper);

/**
 * @internal
 * @memberof SqshSuperblock
 * @brief Cleans up a superblock context.
 *
 * @param[in] superblock The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__superblock_cleanup(struct SqshSuperblock *superblock);

/***************************************
 * archive/compression_options.c
 */

/**
 * @brief The compression options context is used to store the
 * compression options for a specific compression algorithm.
 */
struct SqshCompressionOptions {
	/**
	 * @privatesection
	 */
	uint16_t compression_id;
	struct SqshMetablockIterator metablock;
};

/**
 * @internal
 * @memberof SqshCompressionOptions
 * @brief Initialize the compression options context.
 * @param compression_options the compression options context
 * @param sqsh the Sqsh struct
 *
 * @return 0 on success, less than 0 on error
 */
SQSH_NO_UNUSED int sqsh__compression_options_init(
		struct SqshCompressionOptions *compression_options,
		struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshCompressionOptions
 * @brief Frees the resources used by the compression options context.
 *
 * @param compression_options the compression options context
 *
 * @return 0 on success, less than 0 on error
 */
int sqsh__compression_options_cleanup(
		struct SqshCompressionOptions *compression_options);

/***************************************
 * archive/archive.c
 */

struct SqshArchive {
	/**
	 * @privatesection
	 */
	struct SqshMapManager map_manager;
	struct SqshExtractManager data_extract_manager;
	struct SqshExtractManager metablock_extract_manager;
	struct SqshSuperblock superblock;
	struct SqshIdTable id_table;
	struct SqshExportTable export_table;
	struct SqshXattrTable xattr_table;
	struct SqshFragmentTable fragment_table;
	struct SqshInodeMap inode_map;
	uint8_t initialized;
	struct SqshConfig config;
	sqsh_mutex_t lock;
};

/**
 * @internal
 * @memberof SqshArchive
 * @brief sqsh__init initializes the Sqsh structure.
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
 * @memberof SqshArchive
 * @brief sqsh__archive_data_extract_manager retrieves a SqshExtractManager used
 * for decompressing file and fragment contents.
 *
 * @param archive the SqshArchive to retrieve the SqshExtractManager from.
 * @param data_extract_manager the SqshExtractManager to retrieve.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh__archive_data_extract_manager(
		struct SqshArchive *archive,
		struct SqshExtractManager **data_extract_manager);

/**
 * @internal
 * @memberof SqshArchive
 * @brief sqsh__archive_metablock_extract_manager retrieves a SqshExtractManager
 * used for decompressing metablock contents.
 *
 * @param archive the SqshArchive to retrieve the SqshExtractManager from.
 *
 * @return the SqshExtractManager.
 */
struct SqshExtractManager *
sqsh__archive_metablock_extract_manager(struct SqshArchive *archive);

/**
 * @internal
 * @memberof SqshArchive
 * @brief sqsh__cleanup frees all resources allocated by the Sqsh structure and
 * cleans up the structure.
 *
 * @param sqsh the Sqsh structure to cleanup.
 * @return 0 on success, less than 0 on error.
 */
int sqsh__archive_cleanup(struct SqshArchive *sqsh);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ARCHIVE_PRIVATE_H */
