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
 * @file         compression_options_context.h
 */

#include "../primitive/buffer.h"
#include "../utils.h"

#ifndef COMPRESSION_OPTIONS_CONTEXT_H

#define COMPRESSION_OPTIONS_CONTEXT_H

struct Sqsh;
union SqshCompressionOptions;

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
 * @brief returns a pointer to the compression options data
 * @memberof SqshCompressionOptionsContext
 * @param context the compression options context
 */
const union SqshCompressionOptions *sqsh_compression_options_data(
		const struct SqshCompressionOptionsContext *context);
/**
 * @brief returns the size of the compression options data as returned by
 * @memberof SqshCompressionOptionsContext
 * sqsh_compression_options_data()
 * @param context the compression options context
 */
size_t sqsh_compression_options_size(
		const struct SqshCompressionOptionsContext *context);

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

#endif /* end of include guard COMPRESSION_OPTIONS_CONTEXT_H */
