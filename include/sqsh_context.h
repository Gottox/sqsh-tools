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
 * @file         sqsh_context.h
 */

#ifndef SQSH_CONTEXT_H

#define SQSH_CONTEXT_H

#include "sqsh_mapper.h"
#include "sqsh_primitive.h"

struct Sqsh;
struct SqshXattrIterator;
struct SqshCompressionOptionsContext;
struct SqshFileContext;
struct SqshInodeContext;
struct SqshMetablockContext;
struct SqshMetablockStreamContext;
struct SqshSuperblockContext;
struct SqshTrailingContext;

// context/compression_options_context.c

/**
 * @brief The compression options context is used to store the
 * compression options for a specific compression algorithm.
 */
struct SqshCompressionOptionsContext {
	uint16_t compression_id;
	struct SqshBuffer buffer;
};

/**
 * @brief definitions of gzip strategies
 */
enum SqshGzipStrategies {
	SQSH_GZIP_STRATEGY_NONE = 0x0,
	SQSH_GZIP_STRATEGY_DEFAULT = 0x0001,
	SQSH_GZIP_STRATEGY_FILTERED = 0x0002,
	SQSH_GZIP_STRATEGY_HUFFMAN_ONLY = 0x0004,
	SQSH_GZIP_STRATEGY_RLE = 0x0008,
	SQSH_GZIP_STRATEGY_FIXED = 0x0010,
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
	SQSH_XZ_FILTER_SPARC = 0x0020,
};
/**
 * @brief definitions of lz4 flags
 */
enum SqshLz4Flags {
	SQS_LZ4_FLAG_NONE = 0x0,
	SQSH_LZ4_HIGH_COMPRESSION = 0x0001,
};
/**
 * @brief definitions of Lzo algorithms
 */
enum SqshLzoAlgorithm {
	SQSH_LZO_ALGORITHM_LZO1X_1 = 0x0000,
	SQSH_LZO_ALGORITHM_LZO1X_1_11 = 0x0001,
	SQSH_LZO_ALGORITHM_LZO1X_1_12 = 0x0002,
	SQSH_LZO_ALGORITHM_LZO1X_1_15 = 0x0003,
	SQSH_LZO_ALGORITHM_LZO1X_999 = 0x0004,
};

/**
 * @brief Initialize the compression options context.
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 * @param sqsh the Sqsh struct
 */
SQSH_NO_UNUSED int sqsh_compression_options_init(
		struct SqshCompressionOptionsContext *context, struct Sqsh *sqsh);

/**
 * @brief returns the compression level of gzip
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint32_t sqsh_compression_options_gzip_compression_level(
		const struct SqshCompressionOptionsContext *context);
/**
 * @brief returns the compression window size of gzip
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint16_t sqsh_compression_options_gzip_window_size(
		const struct SqshCompressionOptionsContext *context);
/**
 * @brief returns the compression strategy of gzip
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
enum SqshGzipStrategies sqsh_compression_options_gzip_strategies(
		const struct SqshCompressionOptionsContext *context);

/**
 * @brief returns the dictionary size of xz
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint32_t sqsh_compression_options_xz_dictionary_size(
		const struct SqshCompressionOptionsContext *context);
/**
 * @brief returns the compression options of xz
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
enum SqshXzFilters sqsh_compression_options_xz_filters(
		const struct SqshCompressionOptionsContext *context);

/**
 * @brief returns the version of lz4 used
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint32_t sqsh_compression_options_lz4_version(
		const struct SqshCompressionOptionsContext *context);
/**
 * @brief returns the flags of lz4
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint32_t sqsh_compression_options_lz4_flags(
		const struct SqshCompressionOptionsContext *context);

/**
 * @brief returns the compression level of zstd
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
uint32_t sqsh_compression_options_zstd_compression_level(
		const struct SqshCompressionOptionsContext *context);

/**
 * @brief returns the algorithm of lzo
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
enum SqshLzoAlgorithm sqsh_compression_options_lzo_algorithm(
		const struct SqshCompressionOptionsContext *context);
uint32_t sqsh_compression_options_lzo_compression_level(
		const struct SqshCompressionOptionsContext *context);

/**
 * @brief Frees the resources used by the compression options context.
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
int
sqsh_compression_options_cleanup(struct SqshCompressionOptionsContext *context);

// context/file_context.c

/**
 * @brief The SqshFileContext struct
 *
 * This struct is used to assemble file contents.
 */
struct SqshFileContext {
	struct SqshMapper *mapper;
	struct SqshFragmentTable *fragment_table;
	struct SqshInodeContext *inode;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
	uint64_t seek_pos;
	uint32_t block_size;
};

/**
 * @brief Initializes a SqshFileContext struct.
 * @memberof SqshFileContext
 * @param context The file context to initialize.
 * @param inode The inode context to retrieve the file contents from.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_init(struct SqshFileContext *context, struct SqshInodeContext *inode);

/**
 * @brief Seek to a position in the file content.
 * @memberof SqshFileContext
 * @param context The file context to seek in. If the context buffer
 * already contains data, it will be cleared.
 * @param seek_pos The offset to seek to.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_seek(struct SqshFileContext *context, uint64_t seek_pos);

/**
 * @brief Reads data from the current seek position
 * and writes it to the content buffer.
 * @memberof SqshFileContext
 * @param context The file context to read from.
 * @param size The size of the buffer.
 * @return The number of bytes read on success, less than 0 on error.
 */
int sqsh_file_read(struct SqshFileContext *context, uint64_t size);

/**
 * @brief Gets a pointer to read file content.
 * @memberof SqshFileContext
 * @param context The file context to get the data from.
 * @return A pointer to the data in the file content buffer.
 */
const uint8_t *sqsh_file_data(struct SqshFileContext *context);

/**
 * @brief Gets the size of the file content buffer.
 * @memberof SqshFileContext
 * @param context The file context to get the size from.
 * @return The size of the file content buffer.
 */
uint64_t sqsh_file_size(struct SqshFileContext *context);

/**
 * @brief Frees the resources used by the file context.
 * @memberof SqshFileContext
 * @param context The file context to clean up.
 */
int sqsh_file_cleanup(struct SqshFileContext *context);

// context/metablock_stream_context.c

struct SqshMetablockStreamContext {
	struct Sqsh *sqsh;
	struct SqshBuffer buffer;
	uint64_t base_address;
	uint64_t current_address;
	uint16_t buffer_offset;
};

SQSH_NO_UNUSED int sqsh_metablock_stream_init(
		struct SqshMetablockStreamContext *context, struct Sqsh *sqsh,
		uint64_t address, uint64_t max_address);

SQSH_NO_UNUSED int sqsh_metablock_stream_seek_ref(
		struct SqshMetablockStreamContext *context, uint64_t ref);

SQSH_NO_UNUSED int sqsh_metablock_stream_seek(
		struct SqshMetablockStreamContext *context, uint64_t address_offset,
		uint32_t buffer_offset);

SQSH_NO_UNUSED int sqsh_metablock_stream_more(
		struct SqshMetablockStreamContext *context, uint64_t size);

const uint8_t *
sqsh_metablock_stream_data(const struct SqshMetablockStreamContext *context);

size_t
sqsh_metablock_stream_size(const struct SqshMetablockStreamContext *context);

int sqsh_metablock_stream_cleanup(struct SqshMetablockStreamContext *context);
// context/inode_context.c

#define SQSH_INODE_NO_FRAGMENT 0xFFFFFFFF
#define SQSH_INODE_NO_XATTR 0xFFFFFFFF

enum SqshInodeType {
	SQSH_INODE_TYPE_BASIC_DIRECTORY = 1,
	SQSH_INODE_TYPE_BASIC_FILE = 2,
	SQSH_INODE_TYPE_BASIC_SYMLINK = 3,
	SQSH_INODE_TYPE_BASIC_BLOCK = 4,
	SQSH_INODE_TYPE_BASIC_CHAR = 5,
	SQSH_INODE_TYPE_BASIC_FIFO = 6,
	SQSH_INODE_TYPE_BASIC_SOCKET = 7,
	SQSH_INODE_TYPE_EXTENDED_DIRECTORY = 8,
	SQSH_INODE_TYPE_EXTENDED_FILE = 9,
	SQSH_INODE_TYPE_EXTENDED_SYMLINK = 10,
	SQSH_INODE_TYPE_EXTENDED_BLOCK = 11,
	SQSH_INODE_TYPE_EXTENDED_CHAR = 12,
	SQSH_INODE_TYPE_EXTENDED_FIFO = 13,
	SQSH_INODE_TYPE_EXTENDED_SOCKET = 14,
};

enum SqshInodeContextType {
	SQSH_INODE_TYPE_UNKNOWN = -1,
	// avoid overlapping with the types in ../data/inode_data.h
	SQSH_INODE_TYPE_DIRECTORY = 1 + (1 << 8),
	SQSH_INODE_TYPE_FILE,
	SQSH_INODE_TYPE_SYMLINK,
	SQSH_INODE_TYPE_BLOCK,
	SQSH_INODE_TYPE_CHAR,
	SQSH_INODE_TYPE_FIFO,
	SQSH_INODE_TYPE_SOCKET,
};

/**
 * @brief Inode context.
 */
struct SqshInodeContext {
	struct SqshMetablockStreamContext metablock;
	struct Sqsh *sqsh;
};

/**
 * @brief Initialize the inode context from a inode reference. inode references
 * @memberof SqshInodeContext
 * are descriptors of the physical location of an inode inside the inode table.
 * They are diffrent from the inode number. In doubt use the inode number.
 *
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param inode_ref The inode reference.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_ref(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_ref);
/**
 * @brief Initialize the inode context of the root directory.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_inode_init_root(struct SqshInodeContext *context, struct Sqsh *sqsh);
/**
 * @brief Initialize the inode context from an inode number.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param inode_number The inode number.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_inode_number(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_number);
/**
 * @brief Initialize the inode context from a path.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param path The path the file or directory.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_path(
		struct SqshInodeContext *context, struct Sqsh *sqsh, const char *path);

/**
 * @brief returns whether the inode is an extended structure.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return true if the inode is an extended structure.
 */
bool sqsh_inode_is_extended(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode hard link count.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the amount of hard links to the inode.
 */
uint32_t sqsh_inode_hard_link_count(const struct SqshInodeContext *context);
/**
 * @brief Getter for the file size. 0 if the file has no size.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode type.
 */
uint64_t sqsh_inode_file_size(const struct SqshInodeContext *context);
/**
 * @brief Getter for the permissions of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the permissions of the inode.
 */
uint16_t sqsh_inode_permission(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode number.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode number.
 */
uint32_t sqsh_inode_number(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode modification time.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode modification time.
 */
uint32_t sqsh_inode_modified_time(const struct SqshInodeContext *context);
/**
 * @brief Getter for the start block of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the start block of the file content or UINT64_MAX if the inode
 * is not a file.
 */
uint64_t sqsh_inode_file_blocks_start(const struct SqshInodeContext *context);
/**
 * @brief Getter for the amount of blocks of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the amount of blocks of the file content. If the inode is not a
 * file 0, UINT32_MAX will be returned.
 */
uint32_t sqsh_inode_file_block_count(const struct SqshInodeContext *context);
/**
 * @brief Getter the size of a block of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @param index The index of the block.
 * @return the size of the block with the index.
 */
uint32_t sqsh_inode_file_block_size(
		const struct SqshInodeContext *context, uint32_t index);
/**
 * @brief Checks whether a certain block is compressed.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @param index The index of the block.
 * @return true if the block is compressed, false otherwise.
 */
bool sqsh_inode_file_block_is_compressed(
		const struct SqshInodeContext *context, int index);
/**
 * @brief retrieve the fragment block index. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the fragment block index.
 */
uint32_t
sqsh_inode_file_fragment_block_index(const struct SqshInodeContext *context);
/**
 * @brief retrieve the fragment block offset. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the offset inside of the fragment block.
 */
uint32_t
sqsh_inode_file_fragment_block_offset(const struct SqshInodeContext *context);
/**
 * @brief retrieve the directory block start. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while iterating over the directory entries.
 * @param context The inode context.
 * @return the directory block start.
 */
uint32_t
sqsh_inode_directory_block_start(const struct SqshInodeContext *context);
/**
 * @brief retrieve the directory block offset. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while iterating over the directory entries.
 * @param context The inode context.
 * @return the directory block offset.
 */
uint32_t
sqsh_inode_directory_block_offset(const struct SqshInodeContext *context);
/**
 * @brief returns true if the inode has a fragment block.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return true if the inode has a fragment block, false otherwise.
 */
bool sqsh_inode_file_has_fragment(const struct SqshInodeContext *context);

/**
 * @brief returns the type of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the type of the inode.
 */
enum SqshInodeContextType
sqsh_inode_type(const struct SqshInodeContext *context);

/**
 * @brief returns the target of a symbolic link. Be aware that the returned
 * @memberof SqshInodeContext
 * value is not zero terminated. If you need a zero terminated string use
 * sqsh_inode_symlink_dup().
 * @param context The inode context.
 * @return the name of the inode or NULL if the inode is not a symbolic link.
 */
const char *sqsh_inode_symlink(const struct SqshInodeContext *context);
/**
 * @brief sets a heap allocated, zero terminated string of the target of a
 * @memberof SqshInodeContext
 * symbolic link.
 * @param context The inode context.
 * @param namebuffer a pointer that will be set to the allocated string.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_symlink_dup(
		const struct SqshInodeContext *context, char **namebuffer);
/**
 * @brief returns the length of the target of a symbolic link in bytes.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the length of the target of a symbolic link in bytes or 0 if the
 * inode is not a symbolic link.
 */
uint32_t sqsh_inode_symlink_size(const struct SqshInodeContext *context);

/**
 * @brief returns the device id of the device inode.
 * @param context The inode context.
 * @return the name of the inode or 0 if the inode is not a device.
 */
uint32_t sqsh_inode_device_id(const struct SqshInodeContext *context);

/**
 * @brief returns the uid of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the uid of the inode.
 */
uint32_t sqsh_inode_uid(const struct SqshInodeContext *context);
/**
 * @brief returns the gid of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the gid of the inode.
 */
uint32_t sqsh_inode_gid(const struct SqshInodeContext *context);
/**
 * @brief returns index of the extended attribute inside of the xattr table.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the index of the extended attribute inside of the xattr table.
 */
uint32_t sqsh_inode_xattr_index(const struct SqshInodeContext *context);
SQSH_NO_UNUSED int sqsh_inode_xattr_iterator(
		const struct SqshInodeContext *context,
		struct SqshXattrIterator *iterator);
int sqsh_inode_cleanup(struct SqshInodeContext *context);
/**
 * @brief converts an inode reference into a block index and a block offset
 * @memberof SqshInodeContext
 * @param ref The inode reference.
 * @param block_index a pointer where the block index will be stored.
 * @param offset a pointer where the block offset will be stored.
 */
void
sqsh_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset);
/**
 * @brief converts a block index and a block offset into an inode reference.
 * @memberof SqshInodeContext
 * @param block_index The block index.
 * @param offset The block offset.
 * @return the inode reference.
 */
SQSH_NO_UNUSED uint64_t
sqsh_inode_ref_from_block(uint32_t block_index, uint16_t offset);

// context/path_resolver_context.c

struct SqshPathResolverContext {
	struct Sqsh *sqsh;
};

/**
 * @brief initializes a path resolver context.
 * @memberof SqshPathResolverContext
 * @param context The path resolver context.
 * @param sqsh The sqsh context.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_init(
		struct SqshPathResolverContext *context, struct Sqsh *sqsh);

/**
 * @brief Initialize the inode context from a path.
 * @memberof SqshPathResolverContext
 * @param context The path resolver context.
 * @param inode The inode context to initialize.
 * @param path The path the file or directory.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_path_resolver_resolve(
		struct SqshPathResolverContext *context, struct SqshInodeContext *inode,
		const char *path);

/**
 * @brief cleans up a path resolver context.
 * @memberof SqshPathResolverContext
 * @param context The path resolver context.
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_cleanup(struct SqshPathResolverContext *context);

// context/metablock_context.c

#define SQSH_METABLOCK_BLOCK_SIZE 8192

/**
 * @brief The SqshMetablockContext struct
 *
 * The SqshMetablockContext struct contains all information about a
 * metablock.
 */
struct SqshMetablockContext {
	struct SqshMapping mapping;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
};

/**
 * @brief sqsh_metablock_context_init
 * @param context The SqshMetablockContext to initialize.
 * @param sqsh The Sqsh struct.
 * @param address The address of the metablock.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_metablock_init(
		struct SqshMetablockContext *context, struct Sqsh *sqsh,
		uint64_t address);

uint32_t
sqsh_metablock_compressed_size(const struct SqshMetablockContext *context);

SQSH_NO_UNUSED int sqsh_metablock_to_buffer(
		struct SqshMetablockContext *context, struct SqshBuffer *buffer);

int sqsh_metablock_cleanup(struct SqshMetablockContext *context);

// context/superblock_context.c

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

struct SqshSuperblockContext {
	const struct SqshSuperblock *superblock;
	struct SqshMapping mapping;
};

SQSH_NO_UNUSED int sqsh_superblock_init(
		struct SqshSuperblockContext *context, struct SqshMapper *mapper);

const void *sqsh_superblock_data_from_offset(
		const struct SqshSuperblockContext *context, uint64_t offset);

enum SqshSuperblockCompressionId
sqsh_superblock_compression_id(const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_directory_table_start(
		const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_fragment_table_start(
		const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_inode_count(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_inode_table_start(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_id_table_start(const struct SqshSuperblockContext *context);

uint16_t sqsh_superblock_id_count(const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_export_table_start(const struct SqshSuperblockContext *context);

uint64_t sqsh_superblock_xattr_id_table_start(
		const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_inode_root_ref(const struct SqshSuperblockContext *context);

bool sqsh_superblock_has_fragments(const struct SqshSuperblockContext *context);

bool
sqsh_superblock_has_export_table(const struct SqshSuperblockContext *context);

bool sqsh_superblock_has_compression_options(
		const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_block_size(const struct SqshSuperblockContext *context);

uint32_t
sqsh_superblock_modification_time(const struct SqshSuperblockContext *context);

uint32_t sqsh_superblock_fragment_entry_count(
		const struct SqshSuperblockContext *context);

uint64_t
sqsh_superblock_bytes_used(const struct SqshSuperblockContext *context);

int sqsh_superblock_cleanup(struct SqshSuperblockContext *superblock);

// context/trailing_context.c

struct SqshTrailingContext {
	struct SqshMapping *mapping;
};

int sqsh_trailing_init(struct SqshTrailingContext *context, struct Sqsh *sqsh);

size_t sqsh_trailing_size(struct SqshTrailingContext *context);

const uint8_t *sqsh_trailing_data(struct SqshTrailingContext *context);

int sqsh_trailing_cleanup(struct SqshTrailingContext *context);

#endif /* end of include guard SQSH_CONTEXT_H */
