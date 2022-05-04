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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : hsqs
 * @created     : Friday Apr 30, 2021 10:58:14 CEST
 */

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

struct Hsqs {
	uint32_t error;
	struct HsqsLruHashmap metablock_cache;
	struct HsqsMapper mapper;
	struct HsqsMapper table_mapper;
	struct HsqsMapping table_map;
	struct HsqsSuperblockContext superblock;
	struct HsqsTable id_table;
	struct HsqsTable export_table;
	struct HsqsXattrTable xattr_table;
	struct HsqsFragmentTable fragment_table;
	struct HsqsCompressionOptionsContext compression_options;
	struct HsqsMapping trailing_map;
	uint8_t initialized;
};

HSQS_NO_UNUSED int
hsqs_init(struct Hsqs *hsqs, const uint8_t *buffer, const size_t size);

HSQS_NO_UNUSED int hsqs_open(struct Hsqs *hsqs, const char *path);

int hsqs_request_map(
		struct Hsqs *hsqs, struct HsqsMapping *mapping, uint64_t offset,
		uint64_t size);

struct HsqsSuperblockContext *hsqs_superblock(struct Hsqs *hsqs);

int hsqs_id_table(struct Hsqs *hsqs, struct HsqsTable **id_table);
int hsqs_export_table(struct Hsqs *hsqs, struct HsqsTable **export_table);
int hsqs_fragment_table(
		struct Hsqs *hsqs, struct HsqsFragmentTable **fragment_table);
int hsqs_xattr_table(struct Hsqs *hsqs, struct HsqsXattrTable **xattr_table);
int hsqs_compression_options(
		struct Hsqs *hsqs,
		struct HsqsCompressionOptionsContext **compression_options);
struct HsqsLruHashmap *hsqs_metablock_cache(struct Hsqs *hsqs);
const uint8_t *hsqs_trailing_bytes(struct Hsqs *hsqs);
size_t hsqs_trailing_bytes_size(struct Hsqs *hsqs);
int hsqs_cleanup(struct Hsqs *hsqs);

#endif /* end of include guard HSQS_H */
