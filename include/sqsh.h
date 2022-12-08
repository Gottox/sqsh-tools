/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         sqsh.h
 */

#include "sqsh_common.h"
#include "sqsh_compression.h"
#include "sqsh_context.h"
#include "sqsh_error.h"
#include "sqsh_mapper.h"
#include "sqsh_table.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef SQSH_H

#	define SQSH_H

enum SqshSourceType {
	SQSH_SOURCE_TYPE_PATH,
	SQSH_SOURCE_TYPE_FD,
	SQSH_SOURCE_TYPE_MEMORY,
	SQSH_SOURCE_TYPE_CURL,
};
/**
 * @brief The SqshConfig struct contains all the configuration options for
 * a sqsh session.
 */
struct SqshConfig {
	enum SqshSourceType source_type;
	size_t source_size;
};

/**
 * @brief The SqshContext struct contains all information about the current
 * sqsh session.
 */
struct Sqsh;

/**
 * @brief sqsh_new initializes a sqsh context in heap.
 * @param source the source to retrieve the archive from
 * @param config the configuration for the sqsh context.
 * @param err the error pointer.
 * @return a pointer to the sqsh context or NULL if an error occurred.
 */
SQSH_NO_UNUSED struct Sqsh *
sqsh_new(const void *source, const struct SqshConfig *config, int *err);

/**
 * @brief sqsh_init initializes the Sqsh structure.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to initialize.
 * @param source the source to retrieve the archive from
 * @param config the configuration for the Sqsh session.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_init(
		struct Sqsh *sqsh, const void *source, const struct SqshConfig *config);

/**
 * @brief sqsh_superblock returns the superblock context.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @return the superblock context.
 */
struct SqshSuperblockContext *sqsh_superblock(struct Sqsh *sqsh);

/**
 * @brief sqsh_mapper returns the mapper to retrieve chunks of the sqsh file.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @return the mapper context.
 */
struct SqshMapper *sqsh_mapper(struct Sqsh *sqsh);
/**
 * @brief sqsh_data_compression returns the compression context for data blocks
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @return the compression context.
 */
struct SqshCompression *sqsh_data_compression(struct Sqsh *sqsh);

/**
 * @brief sqsh_data_compression returns the compression context for metadata
 * blocks.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @return the compression context.
 */
struct SqshCompression *sqsh_metablock_compression(struct Sqsh *sqsh);

/**
 * @brief sqsh_id_table returns the id table context.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @param id_table double pointer that will be set to the uid/gid table.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_id_table(struct Sqsh *sqsh, struct SqshTable **id_table);

/**
 * @brief sqsh_export_table returns the export table context. If the archive
 * does not contain an export table, the function returns
 * `-SQSH_ERROR_NO_EXPORT_TABLE`
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @param export_table double pointer that will be set to the export table.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_export_table(struct Sqsh *sqsh, struct SqshTable **export_table);

/**
 * @brief sqsh_fragment_table returns the fragment table context. If the archive
 * does not contain a fragment table, the function returns
 * `-SQSH_ERROR_NO_FRAGMENT_TABLE`.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @param fragment_table double pointer that will be set to the fragment table.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_fragment_table(
		struct Sqsh *sqsh, struct SqshFragmentTable **fragment_table);

/**
 * @brief sqsh_xattr_table returns the xattr table context. If the archive
 * does not contain an xattr table, the function returns
 * `-SQSH_ERROR_NO_XATTR_TABLE`.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @param xattr_table double pointer that will be set to the xattr table.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_xattr_table(struct Sqsh *sqsh, struct SqshXattrTable **xattr_table);

/**
 * @brief sqsh_compression_options returns the compression options context.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure.
 * @param compression_options double pointer that will be set to the
 * compression options context.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_compression_options(
		struct Sqsh *sqsh,
		struct SqshCompressionOptionsContext **compression_options);

/**
 * @brief sqsh_cleanup frees all resources allocated by the Sqsh structure and
 * cleans up the structure.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to cleanup.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_cleanup(struct Sqsh *sqsh);

/**
 * @brief sqsh_free frees up a heap allocated Sqsh structure.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to free.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_free(struct Sqsh *sqsh);

#endif /* end of include guard SQSH_H */
