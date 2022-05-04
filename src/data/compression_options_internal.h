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
 * @file         compression_options_internal.h
 */

#include "../utils.h"
#include "compression_options.h"

#ifndef COMPRESSION_OPTIONS_INTERNAL_H

#define COMPRESSION_OPTIONS_INTERNAL_H

struct HSQS_UNALIGNED HsqsCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};
STATIC_ASSERT(
		sizeof(struct HsqsCompressionOptionsGzip) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS_GZIP);

struct HSQS_UNALIGNED HsqsCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};
STATIC_ASSERT(
		sizeof(struct HsqsCompressionOptionsXz) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS_XZ);

struct HSQS_UNALIGNED HsqsCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};
STATIC_ASSERT(
		sizeof(struct HsqsCompressionOptionsLz4) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS_LZ4);

struct HSQS_UNALIGNED HsqsCompressionOptionsZstd {
	uint32_t compression_level;
};
STATIC_ASSERT(
		sizeof(struct HsqsCompressionOptionsZstd) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS_ZSTD);

struct HSQS_UNALIGNED HsqsCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};
STATIC_ASSERT(
		sizeof(struct HsqsCompressionOptionsLzo) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS_LZO);

union HsqsCompressionOptions {
	struct HsqsCompressionOptionsGzip gzip;
	struct HsqsCompressionOptionsXz xz;
	struct HsqsCompressionOptionsLz4 lz4;
	struct HsqsCompressionOptionsZstd zstd;
	struct HsqsCompressionOptionsLzo lzo;
};
STATIC_ASSERT(
		sizeof(union HsqsCompressionOptions) ==
		HSQS_SIZEOF_COMPRESSION_OPTIONS);

#endif /* end of include guard COMPRESSION_OPTIONS_INTERNAL_H */
