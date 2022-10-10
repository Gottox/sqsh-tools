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

#include "compression/compression.h"
#include "context/compression_options_context.h"
#include "context/superblock_context.h"
#include "error.h"
#include "mapper/mapper.h"
#include "table/fragment_table.h"
#include "table/table.h"
#include "table/xattr_table.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef SQSH_H

#define SQSH_H

struct SqshTrailingContext;

/**
 * @brief The SqshContext struct contains all information about the current
 * sqsh session.
 */
struct Sqsh {
	uint32_t error;
	struct SqshMapper mapper;
	struct SqshCompression data_compression;
	struct SqshCompression metablock_compression;
	struct SqshMapper table_mapper;
	struct SqshMapping table_map;
	struct SqshSuperblockContext superblock;
	struct SqshTable id_table;
	struct SqshTable export_table;
	struct SqshXattrTable xattr_table;
	struct SqshFragmentTable fragment_table;
	struct SqshCompressionOptionsContext compression_options;
	uint8_t initialized;
};

/**
 * @brief sqsh_init initializes the Sqsh structure.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to initialize.
 * @param buffer the buffer to use for the Sqsh structure.
 * @param size the size of the buffer.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_init(struct Sqsh *sqsh, const uint8_t *buffer, const size_t size);

/**
 * @brief sqsh_open opens the sqsh file at the given path.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to initialize.
 * @param path the path to the sqsh file.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_open(struct Sqsh *sqsh, const char *path);

#ifdef CONFIG_CURL
/**
 * @brief sqsh_open_url opens the sqsh file at the given url. Only available if
 * `curl` is enabled.
 * @memberof Sqsh
 * @param sqsh the Sqsh structure to initialize.
 * @param url the url to the sqsh file.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_open_url(struct Sqsh *sqsh, const char *url);
#endif

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

#endif /* end of include guard SQSH_H */
