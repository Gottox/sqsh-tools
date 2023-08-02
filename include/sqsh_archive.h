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

#ifndef SQSH_ARCHIVE_H
#define SQSH_ARCHIVE_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshInodeMap;
struct SqshIdTable;
struct SqshExportTable;
struct SqshXattrTable;
struct SqshFragmentTable;

/***************************************
 * archive/trailing_context.c
 */

struct SqshTrailingContext;

/**
 * @memberof SqshTrailingContext
 * @brief Retrieves the size of the trailing data in a context.
 *
 * @param[in] context The context to retrieve the size from.
 *
 * @return The size of the trailing data in the context.
 */
size_t sqsh_trailing_size(const struct SqshTrailingContext *context);

/**
 * @memberof SqshTrailingContext
 * @brief Retrieves the trailing data in a context.
 *
 * @param[in] context The context to retrieve the data from.
 *
 * @return The trailing data in the context.
 */
const uint8_t *sqsh_trailing_data(const struct SqshTrailingContext *context);

/***************************************
 * archive/inode_map.c
 */

/**
 * @internal
 * @memberof SqshInodeMap
 * @brief Gets the inode reference for a given inode number.
 *
 * @param[in] map The context to use.
 * @param[in] inode_number The inode number to get the reference for.
 *
 * @return The inode reference on success, a negative value on error.
 */
SQSH_NO_UNUSED uint64_t
sqsh_inode_map_get(const struct SqshInodeMap *map, uint64_t inode_number);

/**
 * @internal
 * @memberof SqshInodeMap
 * @brief Sets the inode reference for a given inode number.
 *
 * @param[in] map The context to use.
 * @param[in] inode_number The inode number to set the reference for.
 * @param[in] inode_ref The inode reference to set.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_inode_map_set(
		struct SqshInodeMap *map, uint64_t inode_number, uint64_t inode_ref);

/***************************************
 * archive/superblock_context.c
 */

/**
 * @brief The compression used in the archive.
 */
enum SqshSuperblockCompressionId {
	SQSH_COMPRESSION_GZIP = 1,
	SQSH_COMPRESSION_LZMA = 2,
	SQSH_COMPRESSION_LZO = 3,
	SQSH_COMPRESSION_XZ = 4,
	SQSH_COMPRESSION_LZ4 = 5,
	SQSH_COMPRESSION_ZSTD = 6
};

/**
 * @brief The flags used in the superblock.
 */
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
	SQSH_SUPERBLOCK_UNCOMPRESSED_IDS = 0x0800
};

struct SqshSuperblock;

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the compression id of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the compression id
 * from.
 *
 * @return The compression id of the superblock context.
 */
enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the directory table in a superblock
 * context.
 *
 * @param[in] context The superblock context to retrieve the directory table
 * start offset from.
 *
 * @return The start offset of the directory table in the superblock context.
 */
uint64_t
sqsh_superblock_directory_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the fragment table in a superblock
 * context.
 *
 * @param[in] context The superblock context to retrieve the fragment table
 * start offset from.
 *
 * @return The start offset of the fragment table in the superblock context.
 */
uint64_t
sqsh_superblock_fragment_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the number of inodes in an archive.
 *
 * @param[in] context The superblock context to retrieve the inode count from.
 *
 * @return The number of inodes in the superblock context.
 */
uint32_t sqsh_superblock_inode_count(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the major version of an archive.
 *
 * @param[in] superblock The superblock context to retrieve the major version
 * from.
 *
 * @return The major version of this archive.
 */
uint16_t sqsh_superblock_version_major(const struct SqshSuperblock *superblock);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the minor version of an archive.
 *
 * @param[in] superblock The superblock context to retrieve the minor version
 * from.
 *
 * @return The minor version of this archive.
 */
uint16_t sqsh_superblock_version_minor(const struct SqshSuperblock *superblock);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the inode table in an archive.
 *
 * @param[in] context The superblock context to retrieve the inode table start
 * offset from.
 *
 * @return The start offset of the inode table in the superblock context.
 */
uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the id table in an archive.
 *
 * @param[in] context The superblock context to retrieve the id table start
 * offset from.
 *
 * @return The start offset of the id table in the superblock context.
 */
uint64_t sqsh_superblock_id_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the number of ids in an archive.
 *
 * @param[in] context The superblock context to retrieve the ids count from.
 *
 * @return The number of inodes in the superblock context.
 */
uint16_t sqsh_superblock_id_count(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the export table in an archive.
 *
 * @param[in] context The superblock context to retrieve the export table start
 * offset from.
 *
 * @return The start offset of the export table in the superblock context.
 */
uint64_t
sqsh_superblock_export_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the start offset of the xattr id table in an archive.
 *
 * @param[in] context The superblock context to retrieve the xattr id table
 * start offset from.
 *
 * @return The start offset of the xattr id table in the superblock context.
 */
uint64_t
sqsh_superblock_xattr_id_table_start(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the reference of the root inode in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the root inode
 * reference from.
 *
 * @return The reference of the root inode in the superblock context.
 */
uint64_t sqsh_superblock_inode_root_ref(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Checks if a superblock context has fragment table.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has a fragment table, false otherwise.
 */
bool sqsh_superblock_has_fragments(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Checks if a superblock context has an export table.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has an export table, false otherwise.
 */
bool sqsh_superblock_has_export_table(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Checks if a superblock context has an xattr table.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has an xattr table, false otherwise.
 */
bool sqsh_superblock_has_xattr_table(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Checks if a superblock context has compression options.
 *
 * @param[in] context The superblock context to check.
 *
 * @return True if the superblock context has compression options, false
 * otherwise.
 */
bool
sqsh_superblock_has_compression_options(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the block size of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the block size from.
 *
 * @return The block size of the superblock context.
 */
uint32_t sqsh_superblock_block_size(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the modification time of a superblock context.
 *
 * @param[in] context The superblock context to retrieve the modification time
 * from.
 *
 * @return The modification time of the superblock context.
 */
uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the number of fragment entries in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the fragment entry
 * count from.
 *
 * @return The number of fragment entries in the superblock context.
 */
uint32_t
sqsh_superblock_fragment_entry_count(const struct SqshSuperblock *context);

/**
 * @memberof SqshSuperblock
 * @brief Retrieves the number of bytes used in a superblock context.
 *
 * @param[in] context The superblock context to retrieve the bytes used from.
 *
 * @return The number of bytes used in the superblock context.
 */
uint64_t sqsh_superblock_bytes_used(const struct SqshSuperblock *context);

/***************************************
 * archive/archive.c
 */

/**
 * @brief The SqshConfig struct contains all the configuration options for
 * a sqsh session.
 */
struct SqshConfig {
	/**
	 * @brief represents the offset in bytes where the sqsh_archive
	 * will start.
	 */
	uint64_t archive_offset;

	/**
	 * @brief represents the addressable size of the source in bytes.
	 *
	 * Please note that this is not the size of the archive, but the size of the 
	 * source. So that the maximal size of the archive is `source_size - archive_offset`.
	 *
	 * This field is only used when the source_mapper is unable to determine the size 
	 * of the source itself. This is the case for the following mappers:
	 *
	 * - `sqsh_mapper_impl_static`
	 */
	uint64_t source_size;

	/**
	 * @brief source_mapper is the memory mapper implementation that will be
	 * used to map the archive.
	 *
	 * - `sqsh_mapper_impl_mmap`: the archive will be loaded from a file. The
	 *    source will be interpreted as a file path. this is the default.
	 * - `sqsh_mapper_impl_static`: the archive will be interpreted from a
	 *    static buffer.
	 * - `sqsh_mapper_impl_curl`: the archive will be loaded from a remote
	 *   location. The source will be interpreted as a URL.
	 */
	const struct SqshMemoryMapperImpl *source_mapper;

	/**
	 * @brief the block size used to retrieve chunks of data from the mapper.
	 * If unset or 0, the block size will be determined by the mapper.
	 */
	int mapper_block_size;

	/**
	 * @brief the size of the LRU cache used to cache chunks of data from the
	 * mapper. If unset or 0, the LRU defaults to 32. if set to -1, the LRU
	 * will be disabled.
	 */
	int mapper_lru_size;

	/**
	 * @brief the size of the LRU cache used to cache chunks of data from the
	 * compression algorithm. If unset or 0, the LRU defaults to 128. if set to
	 * -1, the LRU will be disabled.
	 */
	int compression_lru_size;

	/**
	 * @privatesection
	 */
	char _reserved[128];
};

/**
 * @brief The Sqsh struct contains all information about the current
 * sqsh session.
 */
struct SqshArchive;

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
const struct SqshSuperblock *
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
 * @brief Retrieves the id table of a Sqsh instance.
 *
 * @param[in]  archive    The Sqsh instance to retrieve the id table from.
 * @param[out] id_table   Pointer to a struct SqshTable where the id table will
 * be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_archive_id_table(
		struct SqshArchive *archive, struct SqshIdTable **id_table);

/**
 * @memberof SqshArchive
 * @brief Retrieves the export table of a Sqsh instance.
 *
 * @param[in]  archive        The Sqsh instance to retrieve the inode cache
 *                            from.
 * @param[out] inode_map      Pointer to a struct SqshInodeMap where the
 *                            inode cache will be stored.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_archive_inode_map(
		struct SqshArchive *archive, struct SqshInodeMap **inode_map);

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
		struct SqshArchive *archive, struct SqshExportTable **export_table);

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

/***************************************
 * archive/compression_options_context.c
 */

/**
 * @brief definitions of gzip strategies
 */
enum SqshGzipStrategies {
	SQSH_GZIP_STRATEGY_NONE = 0x0,
	SQSH_GZIP_STRATEGY_DEFAULT = 0x0001,
	SQSH_GZIP_STRATEGY_FILTERED = 0x0002,
	SQSH_GZIP_STRATEGY_HUFFMAN_ONLY = 0x0004,
	SQSH_GZIP_STRATEGY_RLE = 0x0008,
	SQSH_GZIP_STRATEGY_FIXED = 0x0010
};
/**
 * @brief definitions xz filters
 */
enum SqshXzFilters {
	SQSH_XZ_FILTER_NONE = 0x0,
	SQSH_XZ_FILTER_X86 = 0x0001,
	SQSH_XZ_FILTER_POWERPC = 0x0002,
	SQSH_XZ_FILTER_IA64 = 0x0004,
	SQSH_XZ_FILTER_ARM = 0x0008,
	SQSH_XZ_FILTER_ARMTHUMB = 0x0010,
	SQSH_XZ_FILTER_SPARC = 0x0020
};
/**
 * @brief definitions of lz4 flags
 */
enum SqshLz4Flags {
	SQS_LZ4_FLAG_NONE = 0x0,
	SQSH_LZ4_HIGH_COMPRESSION = 0x0001
};
/**
 * @brief definitions of Lzo algorithms
 */
enum SqshLzoAlgorithm {
	SQSH_LZO_ALGORITHM_LZO1X_1 = 0x0000,
	SQSH_LZO_ALGORITHM_LZO1X_1_11 = 0x0001,
	SQSH_LZO_ALGORITHM_LZO1X_1_12 = 0x0002,
	SQSH_LZO_ALGORITHM_LZO1X_1_15 = 0x0003,
	SQSH_LZO_ALGORITHM_LZO1X_999 = 0x0004
};

/**
 * @memberof SqshCompressionOptions
 * @brief Initializes a SqshCompressionOptions struct.
 *
 * @param[in] sqsh Sqsh context
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return The Initialized file context
 */
SQSH_NO_UNUSED
struct SqshCompressionOptions *
sqsh_compression_options_new(struct SqshArchive *sqsh, int *err);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression level of gzip
 *
 * @param[in] context the compression options context
 *
 * @return the compression level of gzip
 */
uint32_t sqsh_compression_options_gzip_compression_level(
		const struct SqshCompressionOptions *context);
/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression window size of gzip
 *
 * @param[in] context the compression options context
 *
 * @return the compression window size of gzip
 */
uint16_t sqsh_compression_options_gzip_window_size(
		const struct SqshCompressionOptions *context);
/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression strategy of gzip
 *
 * @param[in] context the compression options context
 *
 * @return the compression strategy of gzip
 */
enum SqshGzipStrategies sqsh_compression_options_gzip_strategies(
		const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the dictionary size of xz
 *
 * @param[in] context the compression options context
 *
 * @return the dictionary size of xz
 */
uint32_t sqsh_compression_options_xz_dictionary_size(
		const struct SqshCompressionOptions *context);
/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression options of xz
 *
 * @param[in] context the compression options context
 *
 * @return the compression options of xz
 */
enum SqshXzFilters sqsh_compression_options_xz_filters(
		const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the version of lz4 used
 *
 * @param[in] context the compression options context
 *
 * @return the version of lz4 used
 */
uint32_t sqsh_compression_options_lz4_version(
		const struct SqshCompressionOptions *context);
/**
 * @memberof SqshCompressionOptions
 * @brief returns the flags of lz4
 *
 * @param[in] context the compression options context
 *
 * @return the flags of lz4
 */
uint32_t sqsh_compression_options_lz4_flags(
		const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression level of zstd
 *
 * @param[in] context the compression options context
 *
 * @return the compression level of zstd
 */
uint32_t sqsh_compression_options_zstd_compression_level(
		const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the algorithm of lzo
 *
 * @param[in] context the compression options context
 *
 * @return the algorithm of lzo
 */
enum SqshLzoAlgorithm sqsh_compression_options_lzo_algorithm(
		const struct SqshCompressionOptions *context);
/**
 * @memberof SqshCompressionOptions
 * @brief returns the compression level of lzo
 *
 * @param[in] context the compression options context
 *
 * @return the compression level of lzo
 */
uint32_t sqsh_compression_options_lzo_compression_level(
		const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief returns the size of the compression options struct
 *
 * @param[in] context the compression options context
 *
 * @return the size of the compression options struct
 */
size_t
sqsh_compression_options_size(const struct SqshCompressionOptions *context);

/**
 * @memberof SqshCompressionOptions
 * @brief Frees a SqshCompressionOptions struct.
 *
 * @param[in] context The file context to free.
 *
 * @return 0 on success, less than 0 on error
 */
int sqsh_compression_options_free(struct SqshCompressionOptions *context);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ARCHIVE_H */
