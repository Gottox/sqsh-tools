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
 * @file         sqsh.h
 */

#include "compression/compression.h"
#include "context/compression_options_context.h"
#include "context/superblock_context.h"
#include "error.h"
#include "mapper/mapper.h"
#include "table/fragment_table.h"
#include "table/table.h"
#include "table/xattr_table.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef HSQS_H

#define HSQS_H

struct SqshTrailingContext;

struct Sqsh {
	uint32_t error;
	struct SqshMapper mapper;
	struct SqshCompression data_compression;
	struct SqshCompression metablock_compression;
	struct SqshMapper table_mapper;
	struct SqshMapping table_map;
	struct SqshSuperblockContext superblock;
	struct SqshTable id_table;
	struct SqshTable export_table;
	struct SqshXattrTable xattr_table;
	struct SqshFragmentTable fragment_table;
	struct SqshCompressionOptionsContext compression_options;
	uint8_t initialized;
};

HSQS_NO_UNUSED int
sqsh_init(struct Sqsh *sqsh, const uint8_t *buffer, const size_t size);

HSQS_NO_UNUSED int sqsh_open(struct Sqsh *sqsh, const char *path);

#ifdef CONFIG_CURL
int sqsh_open_url(struct Sqsh *sqsh, const char *url);
#endif

int sqsh_request_map(
		struct Sqsh *sqsh, struct SqshMapping *mapping, uint64_t offset,
		uint64_t size);

struct SqshSuperblockContext *sqsh_superblock(struct Sqsh *sqsh);
struct SqshCompression *sqsh_data_compression(struct Sqsh *sqsh);
struct SqshCompression *sqsh_metablock_compression(struct Sqsh *sqsh);

int sqsh_id_table(struct Sqsh *sqsh, struct SqshTable **id_table);
int sqsh_export_table(struct Sqsh *sqsh, struct SqshTable **export_table);
int sqsh_fragment_table(
		struct Sqsh *sqsh, struct SqshFragmentTable **fragment_table);
int sqsh_xattr_table(struct Sqsh *sqsh, struct SqshXattrTable **xattr_table);
int sqsh_compression_options(
		struct Sqsh *sqsh,
		struct SqshCompressionOptionsContext **compression_options);
struct SqshLruHashmap *sqsh_metablock_cache(struct Sqsh *sqsh);
HSQS_NO_UNUSED int sqsh_trailing_bytes(
		struct Sqsh *sqsh, struct SqshTrailingContext *trailing_bytes);
int sqsh_cleanup(struct Sqsh *sqsh);

#endif /* end of include guard HSQS_H */
