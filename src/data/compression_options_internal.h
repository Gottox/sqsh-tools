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
 * @file        : compression_options_internal
 * @created     : Friday Sep 17, 2021 11:02:51 CEST
 */

#include "compression_options.h"

#ifndef COMPRESSION_OPTIONS_INTERNAL_H

#define COMPRESSION_OPTIONS_INTERNAL_H

struct SquashCompressionOptionsGzip {
	uint32_t compression_level;
	uint16_t window_size;
	uint16_t strategies;
};

struct SquashCompressionOptionsXz {
	uint32_t dictionary_size;
	uint32_t filters;
};

struct SquashCompressionOptionsLz4 {
	uint32_t version;
	uint32_t flags;
};

struct SquashCompressionOptionsZstd {
	uint32_t compression_level;
};

struct SquashCompressionOptionsLzo {
	uint32_t algorithm;
	uint32_t compression_level;
};

union SquashCompressionOptions {
	struct SquashCompressionOptionsGzip gzip;
	struct SquashCompressionOptionsXz xz;
	struct SquashCompressionOptionsLz4 lz4;
	struct SquashCompressionOptionsZstd zstd;
	struct SquashCompressionOptionsLzo lzo;
};

#endif /* end of include guard COMPRESSION_OPTIONS_INTERNAL_H */