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
 * @file         sqsh.h
 */

#ifndef SQSH_H
#define SQSH_H

#include "sqsh_common.h"

struct SqshTable;
struct SqshFragmentTable;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The SqshConfig struct contains all the configuration options for
 * a sqsh session.
 */

struct SqshConfig {
#define SQSH_CONFIG_FIELDS \
	uint64_t source_size; \
	const struct SqshMemoryMapperImpl *source_mapper; \
	size_t mapper_block_size;

	SQSH_CONFIG_FIELDS
	uint8_t _reserved[128 - sizeof(struct {SQSH_CONFIG_FIELDS})];
#undef SQSH_CONFIG_FIELDS
};

struct Sqsh;
struct SqshXattrTable;

/**
 * @memberof Sqsh
 * @brief sqsh_new initializes a sqsh context in heap.
 *
 * @param[in] source the source to retrieve the archive from
 * @param[in] config the configuration for the sqsh context.
 * @param[out] err   Pointer to an int where the error code will be stored.
 *
 * @return a pointer to the sqsh context or NULL if an error occurred.
 */
SQSH_NO_UNUSED struct Sqsh *
sqsh_new(const void *source, const struct SqshConfig *config, int *err);

/**
 * @memberof Sqsh
 * @brief sqsh_superblock returns the configuration object of the sqsh context.
 *
 * @param[in] sqsh the Sqsh structure.
 *
 * @return the superblock context.
 */
const struct SqshConfig *sqsh_config(const struct Sqsh *sqsh);

/**
 * @memberof Sqsh
 * @brief sqsh_superblock returns the superblock context.
 *
 * @param[in] sqsh the Sqsh structure.
 *
 * @return the superblock context.
 */
const struct SqshSuperblockContext *sqsh_superblock(const struct Sqsh *sqsh);

/**
 * @memberof Sqsh
 * @brief sqsh_mapper returns the map manager to retrieve chunks of the sqsh
 * file.
 *
 * @param[in] sqsh the Sqsh structure.
 *
 * @return the mapper context.
 */
struct SqshMapManager *sqsh_map_manager(struct Sqsh *sqsh);

/**
 * @memberof Sqsh
 * @brief sqsh_compression_data returns the compression context for data blocks
 *
 * @param[in] sqsh the Sqsh structure.
 *
 * @return the compression context.
 */
const struct SqshCompression *sqsh_compression_data(const struct Sqsh *sqsh);

/**
 * @memberof Sqsh
 * @brief sqsh_compression_data returns the compression context for metadata
 * blocks.
 *
 * @param[in] sqsh the Sqsh structure.
 *
 * @return the compression context.
 */
const struct SqshCompression *
sqsh_compression_metablock(const struct Sqsh *sqsh);

/**
 * @memberof Sqsh
 * @brief Retrieves the id table of a Sqsh instance.
 *
 * @param[in]  sqsh       The Sqsh instance to retrieve the id table from.
 * @param[out] id_table   Pointer to a struct SqshTable where the id table will
 * be stored.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_id_table(struct Sqsh *sqsh, struct SqshTable **id_table);

/**
 * @memberof Sqsh
 * @brief Retrieves the export table of a Sqsh instance.
 *
 * @param[in]  sqsh           The Sqsh instance to retrieve the export table
 *                            from.
 * @param[out] export_table   Pointer to a struct SqshTable where the export
 *                            table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_export_table(struct Sqsh *sqsh, struct SqshTable **export_table);

/**
 * @memberof Sqsh
 * @brief Retrieves the fragment table of a Sqsh instance.
 *
 * @param[in]  sqsh             The Sqsh instance to retrieve the fragment table
 *                              from.
 * @param[out] fragment_table   Pointer to a struct SqshTable where the export
 *                              table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_fragment_table(
		struct Sqsh *sqsh, struct SqshFragmentTable **fragment_table);

/**
 * @memberof Sqsh
 * @brief Retrieves the xattr table of a Sqsh instance.
 *
 * @param[in]  sqsh          The Sqsh instance to retrieve the xattr table
 *                           from.
 * @param[out] xattr_table   Pointer to a struct SqshTable where the export
 *                           table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_xattr_table(struct Sqsh *sqsh, struct SqshXattrTable **xattr_table);

/**
 * @memberof Sqsh
 * @brief Frees the resources used by a Sqsh instance.
 *
 * @param[in] sqsh The Sqsh instance to free.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_free(struct Sqsh *sqsh);

#ifdef __cplusplus
}
#endif
#endif // SQSH_H
