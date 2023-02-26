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

#ifndef SQSH_CONTEXT_H
#define SQSH_CONTEXT_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// context/compression_options_context.c

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
 * @memberof SqshCompressionOptionsContext
 * @brief Initializes a SqshCompressionOptionsContext struct.
 *
 * @param[in] sqsh Sqsh context
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return The Initialized file context
 */
SQSH_NO_UNUSED
struct SqshCompressionOptionsContext *
sqsh_compression_options_new(struct SqshArchive *sqsh, int *err);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression level of gzip
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_gzip_compression_level(
		const struct SqshCompressionOptionsContext *context);
/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression window size of gzip
 *
 * @param[in] context the compression options context
 */
uint16_t sqsh_compression_options_gzip_window_size(
		const struct SqshCompressionOptionsContext *context);
/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression strategy of gzip
 *
 * @param[in] context the compression options context
 */
enum SqshGzipStrategies sqsh_compression_options_gzip_strategies(
		const struct SqshCompressionOptionsContext *context);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the dictionary size of xz
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_xz_dictionary_size(
		const struct SqshCompressionOptionsContext *context);
/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression options of xz
 *
 * @param[in] context the compression options context
 */
enum SqshXzFilters sqsh_compression_options_xz_filters(
		const struct SqshCompressionOptionsContext *context);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the version of lz4 used
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_lz4_version(
		const struct SqshCompressionOptionsContext *context);
/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the flags of lz4
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_lz4_flags(
		const struct SqshCompressionOptionsContext *context);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression level of zstd
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_zstd_compression_level(
		const struct SqshCompressionOptionsContext *context);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the algorithm of lzo
 *
 * @param[in] context the compression options context
 */
enum SqshLzoAlgorithm sqsh_compression_options_lzo_algorithm(
		const struct SqshCompressionOptionsContext *context);
/**
 * @memberof SqshCompressionOptionsContext
 * @brief returns the compression level of lzo
 *
 * @param[in] context the compression options context
 */
uint32_t sqsh_compression_options_lzo_compression_level(
		const struct SqshCompressionOptionsContext *context);

/**
 * @memberof SqshCompressionOptionsContext
 * @brief Frees a SqshCompressionOptionsContext struct.
 *
 * @param[in] context The file context to free.
 */
int
sqsh_compression_options_free(struct SqshCompressionOptionsContext *context);

////////////////////////////////////////
// context/path_resolver_context.c

struct SqshPathResolverContext;

/**
 * @memberof SqshPathResolverContext
 * @brief initializes a path resolver context in heap
 *
 * @param[in]  sqsh The sqsh context.
 * @param[out] err  Pointer to an int where the error code will be stored.
 *
 * @return The Initialized path resolver context
 */
struct SqshPathResolverContext *
sqsh_path_resolver_new(struct SqshArchive *sqsh, int *err);

/**
 * @memberof SqshPathResolverContext
 * @brief Initialize the inode context from a path.
 *
 * @param[in] context The path resolver context.
 * @param[in] path The path the file or directory.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return an inode context on success, NULL on error
 */
SQSH_NO_UNUSED struct SqshInodeContext *sqsh_path_resolver_resolve(
		struct SqshPathResolverContext *context, const char *path, int *err);

/**
 * @memberof SqshPathResolverContext
 * @brief cleans up a path resolver context and frees the memory.
 *
 * @param[in] context The path resolver context.
 *
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_path_resolver_free(struct SqshPathResolverContext *context);

////////////////////////////////////////
// context/trailing_context.c

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

#ifdef __cplusplus
}
#endif
#endif // SQSH_CONTEXT_H
