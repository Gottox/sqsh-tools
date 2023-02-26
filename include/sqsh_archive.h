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
 * @file         sqsh_archive.h
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
	int mapper_block_size; \
	int mapper_lru_size;

	SQSH_CONFIG_FIELDS
	uint8_t _reserved[128 - sizeof(struct {SQSH_CONFIG_FIELDS})];
#undef SQSH_CONFIG_FIELDS
};

struct SqshArchive;
struct SqshXattrTable;

/**
 * @memberof SqshArchive
 * @brief initializes a archive context in heap.
 *
 * @param[in] source the source to retrieve the archive from
 * @param[in] config the configuration for the archive context.
 * @param[out] err   Pointer to an int where the error code will be stored.
 *
 * @return a pointer to the archive context or NULL if an error occurred.
 */
SQSH_NO_UNUSED struct SqshArchive *
sqsh_archive_new(const void *source, const struct SqshConfig *config, int *err);

/**
 * @memberof SqshArchive
 * @brief sqsh_superblock returns the configuration object of the archive
 * context.
 *
 * @param[in] archive the SqshArchive structure.
 *
 * @return the superblock context.
 */
const struct SqshConfig *sqsh_archive_config(const struct SqshArchive *archive);

/**
 * @memberof SqshArchive
 * @brief sqsh_superblock returns the superblock context.
 *
 * @param[in] archive the Sqsh structure.
 *
 * @return the superblock context.
 */
const struct SqshSuperblockContext *
sqsh_archive_superblock(const struct SqshArchive *archive);

/**
 * @memberof SqshArchive
 * @brief sqsh_mapper returns the map manager to retrieve chunks of the archive
 * file.
 *
 * @param[in] archive the Sqsh structure.
 *
 * @return the mapper context.
 */
struct SqshMapManager *sqsh_archive_map_manager(struct SqshArchive *archive);

/**
 * @memberof SqshArchive
 * @brief sqsh_compression_data returns the compression context for data blocks
 *
 * @param[in] archive the Sqsh structure.
 *
 * @return the compression context.
 */
const struct SqshCompression *
sqsh_archive_compression_data(const struct SqshArchive *archive);

/**
 * @memberof SqshArchive
 * @brief sqsh_compression_data returns the compression context for metadata
 * blocks.
 *
 * @param[in] archive the Sqsh structure.
 *
 * @return the compression context.
 */
const struct SqshCompression *
sqsh_archive_compression_metablock(const struct SqshArchive *archive);

/**
 * @memberof SqshArchive
 * @brief Retrieves the id table of a Sqsh instance.
 *
 * @param[in]  archive    The Sqsh instance to retrieve the id table from.
 * @param[out] id_table   Pointer to a struct SqshTable where the id table will
 * be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh_archive_id_table(struct SqshArchive *archive, struct SqshTable **id_table);

/**
 * @memberof SqshArchive
 * @brief Retrieves the export table of a Sqsh instance.
 *
 * @param[in]  archive           The Sqsh instance to retrieve the export table
 *                            from.
 * @param[out] export_table   Pointer to a struct SqshTable where the export
 *                            table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_archive_export_table(
		struct SqshArchive *archive, struct SqshTable **export_table);

/**
 * @memberof SqshArchive
 * @brief Retrieves the fragment table of a Sqsh instance.
 *
 * @param[in]  archive          The Sqsh instance to retrieve the fragment table
 *                              from.
 * @param[out] fragment_table   Pointer to a struct SqshTable where the export
 *                              table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_archive_fragment_table(
		struct SqshArchive *archive, struct SqshFragmentTable **fragment_table);

/**
 * @memberof SqshArchive
 * @brief Retrieves the xattr table of a Sqsh instance.
 *
 * @param[in]  archive       The Sqsh instance to retrieve the xattr table
 *                           from.
 * @param[out] xattr_table   Pointer to a struct SqshTable where the export
 *                           table will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_archive_xattr_table(
		struct SqshArchive *archive, struct SqshXattrTable **xattr_table);

/**
 * @memberof SqshArchive
 * @brief Frees the resources used by a Sqsh instance.
 *
 * @param[in] archive The Sqsh instance to free.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_archive_free(struct SqshArchive *archive);

////////////////////////////////////////
// archive/superblock_context.c

enum SqshSuperblockCompressionId {
	SQSH_COMPRESSION_GZIP = 1,
	SQSH_COMPRESSION_LZMA = 2,
	SQSH_COMPRESSION_LZO = 3,
	SQSH_COMPRESSION_XZ = 4,
	SQSH_COMPRESSION_LZ4 = 5,
	SQSH_COMPRESSION_ZSTD = 6,
};

enum SqshSuperblockFlags {
	SQSH_SUPERBLOCK_UNCOMPRESSED_INODES = 0x0001,
	SQSH_SUPERBLOCK_UNCOMPRESSED_DATA = 0x0002,
	SQSH_SUPERBLOCK_CHECK = 0x0004,
	SQSH_SUPERBLOCK_UNCOMPRESSED_FRAGMENTS = 0x0008,
	SQSH_SUPERBLOCK_NO_FRAGMENTS = 0x0010,
	SQSH_SUPERBLOCK_ALWAYS_FRAGMENTS = 0x0020,
	SQSH_SUPERBLOCK_DUPLICATES = 0x0040,
	SQSH_SUPERBLOCK_EXPORTABLE = 0x0080,
	SQSH_SUPERBLOCK_UNCOMPRESSED_XATTRS = 0x0100,
	SQSH_SUPERBLOCK_NO_XATTRS = 0x0200,
	SQSH_SUPERBLOCK_COMPRESSOR_OPTIONS = 0x0400,
	SQSH_SUPERBLOCK_UNCOMPRESSED_IDS = 0x0800,
};

struct SqshSuperblockContext;

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the compression id of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the compression id
 * from.
 *
 * @return The compression id of the superblock context.
 */
enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the directory table in a superblock
 * context.
 *
 * @param[in] context The superblock context to retrieve the directory table
 * start offset from.
 *
 * @return The start offset of the directory table in the superblock context.
 */
uint64_t sqsh_superblock_directory_table_start(
		const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the fragment table in a superblock
 * context.
 *
 * @param[in] context The superblock context to retrieve the fragment table
 * start offset from.
 *
 * @return The start offset of the fragment table in the superblock context.
 */
uint64_t sqsh_superblock_fragment_table_start(
		const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the number of inodes in an archive.
 *
 * @param[in] context The superblock context to retrieve the inode count from.
 *
 * @return The number of inodes in the superblock context.
 */
uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the inode table in an archive.
 *
 * @param[in] context The superblock context to retrieve the inode table start
 * offset from.
 *
 * @return The start offset of the inode table in the superblock context.
 */
uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the id table in an archive.
 *
 * @param[in] context The superblock context to retrieve the id table start
 * offset from.
 *
 * @return The start offset of the id table in the superblock context.
 */
uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the number of ids in an archive.
 *
 * @param[in] context The superblock context to retrieve the ids count from.
 *
 * @return The number of inodes in the superblock context.
 */
uint16_t sqsh_superblock_id_count(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the export table in an archive.
 *
 * @param[in] context The superblock context to retrieve the export table start
 * offset from.
 *
 * @return The start offset of the export table in the superblock context.
 */
uint64_t
sqsh_superblock_export_table_start(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the start offset of the xattr id table in an archive.
 *
 * @param[in] context The superblock context to retrieve the xattr id table
 * start offset from.
 *
 * @return The start offset of the xattr id table in the superblock context.
 */
uint64_t sqsh_superblock_xattr_id_table_start(
		const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the reference of the root inode in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the root inode
 * reference from.
 *
 * @return The reference of the root inode in the superblock context.
 */
uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Checks if a superblock context has fragment table.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has a fragment table, false otherwise.
 */
bool sqsh_superblock_has_fragments(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Checks if a superblock context has an export table.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has an export table, false otherwise.
 */
bool
sqsh_superblock_has_export_table(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Checks if a superblock context has compression options.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has compression options, false
 * otherwise.
 */
bool sqsh_superblock_has_compression_options(
		const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the block size of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the block size from.
 *
 * @return The block size of the superblock context.
 */
uint32_t
sqsh_superblock_block_size(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the modification time of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the modification time
 * from.
 *
 * @return The modification time of the superblock context.
 */
uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the number of fragment entries in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the fragment entry
 * count from.
 *
 * @return The number of fragment entries in the superblock context.
 */
uint32_t sqsh_superblock_fragment_entry_count(
		const struct SqshSuperblockContext *context);

/**
 * @memberof SqshSuperblockContext
 * @brief Retrieves the number of bytes used in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the bytes used from.
 *
 * @return The number of bytes used in the superblock context.
 */
uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblockContext *context);

#ifdef __cplusplus
}
#endif
#endif // SQSH_H
