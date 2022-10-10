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
 * @file         compression_options.h
 */

#include <stdint.h>

#ifndef SQSH__COMPRESSION_OPTIONS_H

#define SQSH__COMPRESSION_OPTIONS_H

#define SQSH_SIZEOF_COMPRESSION_OPTIONS_GZIP 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_XZ 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_LZ4 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_ZSTD 4
#define SQSH_SIZEOF_COMPRESSION_OPTIONS_LZO 8
#define SQSH_SIZEOF_COMPRESSION_OPTIONS 8

struct SqshCompressionOptionsGzip;

struct SqshCompressionOptionsXz;

struct SqshCompressionOptionsLz4;

struct SqshCompressionOptionsZstd;

struct SqshCompressionOptionsLzo;

union SqshCompressionOptions;

uint32_t sqsh_data_compression_options_gzip_compression_level(
		const union SqshCompressionOptions *options);
uint16_t sqsh_data_compression_options_gzip_window_size(
		const union SqshCompressionOptions *options);
uint16_t sqsh_data_compression_options_gzip_strategies(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_xz_dictionary_size(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_xz_filters(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_lz4_version(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_lz4_flags(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_zstd_compression_level(
		const union SqshCompressionOptions *options);

uint32_t sqsh_data_compression_options_lzo_algorithm(
		const union SqshCompressionOptions *options);
uint32_t sqsh_data_compression_options_lzo_compression_level(
		const union SqshCompressionOptions *options);

#endif /* end of include guard SQSH__COMPRESSION_OPTIONS_H */
