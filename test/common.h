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
 * @file         common.h
 */

#ifndef COMMON_H
#define COMMON_H

#include "../include/sqsh_archive.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#	define UINT16_BYTES(x) (uint8_t)(x), (uint8_t)((x) >> 8)
#	define UINT32_BYTES(x) \
		UINT16_BYTES((uint16_t)(x)), UINT16_BYTES((uint16_t)((x) >> 8))
#	define UINT64_BYTES(x) \
		UINT32_BYTES((uint32_t)(x)), UINT32_BYTES((uint32_t)((x) >> 8))
#else
#	define UINT16_BYTES(x) (uint8_t)((x) >> 8), (uint8_t)(x)
#	define UINT32_BYTES(x) \
		UINT16_BYTES((uint16_t)((x) >> 8)), UINT16_BYTES((uint16_t)(x))
#	define UINT64_BYTES(x) \
		UINT32_BYTES((uint16_t)((x) >> 8)), UINT16_BYTES((uint16_t)(x))
#endif

#define METABLOCK_HEADER(c, s) UINT16_BYTES(((c) ? 0 : 0x8000) + (s))

#define ZLIB_ABCD \
	0x78, 0x9c, 0x4b, 0x4c, 0x4a, 0x4e, 0x01, 0x00, 0x03, 0xd8, 0x01, 0x8b
#define ZLIB_EFGH \
	0x78, 0x9c, 0x4b, 0x4d, 0x4b, 0xcf, 0x00, 0x00, 0x04, 0x00, 0x01, 0x9b

#define CHUNK_SIZE(...) sizeof((uint8_t[]){__VA_ARGS__})

#define DEFAULT_MAPPER sqsh_mapper_impl_static
// We're using a ridiculously small block size to
// test the mappers ability to handle small blocks.
#define DEFAULT_BLOCK_SIZE 1

#define DEFAULT_CONFIG(s) \
	(struct SqshConfig) { \
		.source_size = (s), .source_mapper = DEFAULT_MAPPER, \
		.mapper_block_size = DEFAULT_BLOCK_SIZE, \
	}

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

uint8_t *
mk_stub(struct Sqsh *sqsh, uint8_t *payload, size_t payload_size,
		size_t *target_size);

#endif /* !COMMON_H */
