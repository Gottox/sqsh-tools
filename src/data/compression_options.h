/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression_options
 * @created     : Monday Sep 06, 2021 11:25:09 CEST
 */

#include <stdint.h>

#ifndef HSQS__COMPRESSION_OPTIONS_H

#define HSQS__COMPRESSION_OPTIONS_H

#define HSQS_SIZEOF_COMPRESSION_OPTIONS_GZIP 8
#define HSQS_SIZEOF_COMPRESSION_OPTIONS_XZ 8
#define HSQS_SIZEOF_COMPRESSION_OPTIONS_LZ4 8
#define HSQS_SIZEOF_COMPRESSION_OPTIONS_ZSTD 4
#define HSQS_SIZEOF_COMPRESSION_OPTIONS_LZO 8
#define HSQS_SIZEOF_COMPRESSION_OPTIONS 8

struct HsqsCompressionOptionsGzip;

struct HsqsCompressionOptionsXz;

struct HsqsCompressionOptionsLz4;

struct HsqsCompressionOptionsZstd;

struct HsqsCompressionOptionsLzo;

union HsqsCompressionOptions;

uint32_t hsqs_compression_options_gzip_compression_level(
		const union HsqsCompressionOptions *options);
uint16_t hsqs_compression_options_gzip_window_size(
		const union HsqsCompressionOptions *options);
uint16_t hsqs_compression_options_gzip_strategies(
		const union HsqsCompressionOptions *options);

uint32_t hsqs_compression_options_xz_dictionary_size(
		const union HsqsCompressionOptions *options);
uint32_t hsqs_compression_options_xz_filters(
		const union HsqsCompressionOptions *options);

uint32_t hsqs_compression_options_lz4_version(
		const union HsqsCompressionOptions *options);
uint32_t
hsqs_compression_options_lz4_flags(const union HsqsCompressionOptions *options);

uint32_t hsqs_compression_options_zstd_compression_level(
		const union HsqsCompressionOptions *options);

uint32_t hsqs_compression_options_lzo_algorithm(
		const union HsqsCompressionOptions *options);
uint32_t hsqs_compression_options_lzo_compression_level(
		const union HsqsCompressionOptions *options);

#endif /* end of include guard HSQS__COMPRESSION_OPTIONS_H */
