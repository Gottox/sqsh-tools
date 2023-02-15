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
 * @file         sqsh_context.h
 */

#ifndef SQSH_CONTEXT_PRIVATE_H
#define SQSH_CONTEXT_PRIVATE_H

#include "sqsh_context.h"
#include "sqsh_mapper.h"
#include "sqsh_primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Sqsh;

////////////////////////////////////////
// context/superblock_context.c

struct SqshSuperblockContext {
	/**
	 * @privatesection
	 */
	const struct SqshDataSuperblock *superblock;
	struct SqshMapping mapping;
};

/**
 * @internal
 *
 * @brief Initializes a superblock context.
 *
 * @param[out] context The context to initialize.
 * @param[in]  mapper  The mapper to use for the superblock.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__superblock_init(
		struct SqshSuperblockContext *context, struct SqshMapper *mapper);

/**
 * @brief Cleans up a superblock context.
 *
 * @param[in] superblock The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__superblock_cleanup(struct SqshSuperblockContext *superblock);

////////////////////////////////////////
// context/compression_options_context.c

/**
 * @brief The compression options context is used to store the
 * compression options for a specific compression algorithm.
 */
struct SqshCompressionOptionsContext {
	/**
	 * @privatesection
	 */
	uint16_t compression_id;
	struct SqshBuffer buffer;
};

/**
 * @brief Initialize the compression options context.
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 * @param sqsh the Sqsh struct
 */
SQSH_NO_UNUSED int sqsh__compression_options_init(
		struct SqshCompressionOptionsContext *context, struct Sqsh *sqsh);

/**
 * @brief Frees the resources used by the compression options context.
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
int sqsh__compression_options_cleanup(
		struct SqshCompressionOptionsContext *context);

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
	struct SqshMapper *mapper;
	struct SqshFragmentTable *fragment_table;
	const struct SqshInodeContext *inode;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
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

////////////////////////////////////////
// context/metablock_context.c

#define SQSH_METABLOCK_BLOCK_SIZE 8192

/**
 * @brief The SqshMetablockContext struct
 *
 * The SqshMetablockContext struct contains all information about a
 * metablock.
 */
struct SqshMetablockContext {
	/**
	 * @privatesection
	 */
	struct SqshMapping mapping;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
};

/**
 * @memberof SqshMetablockContext
 * @brief Initializes a metablock context with a SQSH context and an address.
 *
 * @param[out] context The metablock context to initialize.
 * @param[in]  sqsh The SQSH context to use for the metablock.
 * @param[in]  address The starting offset of the metablock in blocks.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__metablock_init(
		struct SqshMetablockContext *context, struct Sqsh *sqsh,
		uint64_t address);

/**
 * @internal
 * @memberof SqshMetablockContext
 * @brief Retrieves the compressed size of the metablock.
 *
 * @param[in] context The metablock context.
 *
 * @return The compressed size of the metablock.
 */
uint32_t
sqsh__metablock_compressed_size(const struct SqshMetablockContext *context);

/**
 * @internal
 * @memberof SqshMetablockContext
 * @brief Writes the metablock to a buffer.
 *
 * @param[in]  context The metablock context.
 * @param[out] buffer The buffer to write the metablock to.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_to_buffer(
		struct SqshMetablockContext *context, struct SqshBuffer *buffer);

/**
 * @internal
 * @memberof SqshMetablockContext
 * @brief Cleans up a metablock context.
 *
 * @param[in] context The metablock context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__metablock_cleanup(struct SqshMetablockContext *context);

////////////////////////////////////////
// context/metablock_stream_context.c

struct SqshMetablockStreamContext {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
	struct SqshBuffer buffer;
	uint64_t base_address;
	uint64_t current_address;
	uint16_t buffer_offset;
};

/**
 * @internal
 * @memberof SqshMetablockStreamContext
 * @brief Initializes a metablock stream context.
 *
 * @param[out] context The metablock stream context to initialize.
 * @param[in] sqsh The sqsh context.
 * @param[in] address The starting address of the stream.
 * @param[in] max_address The maximum address of the stream.
 *
 * @return 0 on success, negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_stream_init(
		struct SqshMetablockStreamContext *context, struct Sqsh *sqsh,
		uint64_t address, uint64_t max_address);

/**
 * @internal
 * @memberof SqshMetablockStreamContext
 * @brief Seeks to a specific metablock reference in the stream.
 *
 * @param[in,out] context The metablock stream context.
 * @param[in] ref The metablock reference to seek to.
 *
 * @return 0 on success, negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_stream_seek_ref(
		struct SqshMetablockStreamContext *context, uint64_t ref);

/**
 * @internal
 * @memberof SqshMetablockStreamContext
 * @brief Seeks to a specific offset in the stream.
 *
 * @param[in,out] context The metablock stream context.
 * @param[in] address_offset The address offset to seek to.
 * @param[in] buffer_offset The buffer offset to seek to.
 *
 * @return 0 on success, negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_stream_seek(
		struct SqshMetablockStreamContext *context, uint64_t address_offset,
		uint32_t buffer_offset);

/**
 * @internal
 * @memberof SqshMetablockStreamContext
 * @brief fills the buffer of the stream context to a specific size.
 *
 * @param[in,out] context The metablock stream context.
 * @param[in] size The of the
 *
 * @return 0 on success, negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_stream_more(
		struct SqshMetablockStreamContext *context, uint64_t size);

/**
 * @internal
 * @brief Retrieve the current data buffer of the metablock stream
 *
 * @param[in] context The metablock stream context
 *
 * @return Pointer to the current data buffer
 */
const uint8_t *
sqsh__metablock_stream_data(const struct SqshMetablockStreamContext *context);

/**
 * @internal
 * @brief Retrieve the current size of the data buffer of the metablock stream
 *
 * @param[in] context The metablock stream context
 *
 * @return The current size of the data buffer
 */
size_t
sqsh__metablock_stream_size(const struct SqshMetablockStreamContext *context);

/**
 * @internal
 * @brief Clean up the metablock stream context
 *
 * @param[in] context The metablock stream context
 *
 * @return 0 on success, a negative value on error
 */
int sqsh__metablock_stream_cleanup(struct SqshMetablockStreamContext *context);

////////////////////////////////////////
// context/inode_context.c

struct SqshInodeContext {
	/**
	 * @privatesection
	 */
	struct SqshMetablockStreamContext metablock;
	struct Sqsh *sqsh;
};

/**
 * @internal
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
SQSH_NO_UNUSED int sqsh__inode_init(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_ref);
/**
 * @internal
 * @brief cleans up the inode context.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return int 0 on success, less than 0 on error.
 */
int sqsh__inode_cleanup(struct SqshInodeContext *context);

////////////////////////////////////////
// context/path_resolver_context.c

struct SqshPathResolverContext {
	/**
	 * @privatesection
	 */
	struct Sqsh *sqsh;
};

/**
 * @internal
 * @memberof SqshPathResolverContext
 * @brief initializes a path resolver context.
 *
 * @param[out] context The path resolver context.
 * @param[in] sqsh The sqsh context.
 *
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__path_resolver_init(
		struct SqshPathResolverContext *context, struct Sqsh *sqsh);

/**
 * @internal
 * @brief cleans up a path resolver context.
 * @memberof SqshPathResolverContext
 *
 * @param[in] context The path resolver context.
 *
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_cleanup(struct SqshPathResolverContext *context);

////////////////////////////////////////
// context/trailing_context.c

struct SqshTrailingContext {
	/**
	 * @privatesection
	 */
	struct SqshMapCursor cursor;
};

/**
 * @memberof SqshTrailingContext
 * @brief Initializes a trailing context.
 *
 * @param[out] context The context to initialize.
 * @param[in]  sqsh The Sqsh instance to use for the context.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__trailing_init(struct SqshTrailingContext *context, struct Sqsh *sqsh);

/**
 * @memberof SqshTrailingContext
 * @brief Cleans up a trailing context.
 *
 * @param[in] context The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__trailing_cleanup(struct SqshTrailingContext *context);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_CONTEXT_PRIVATE_H */
