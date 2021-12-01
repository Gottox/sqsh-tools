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
 * @file        : metablock
 * @created     : Saturday Sep 04, 2021 23:13:19 CEST
 */

#include "../compression/buffer.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef HSQS_EXTRACT_H

#define HSQS_EXTRACT_H

#define HSQS_METABLOCK_BLOCK_SIZE 8192

struct HsqsMetablock;

struct HsqsMetablockContext {
	const struct HsqsSuperblockContext *superblock;
	struct HsqsBuffer buffer;
	off_t start_block;
	off_t index;
	off_t offset;
};

// DEPRECATED:
HSQS_NO_UNUSED int hsqs_metablock_from_offset(
		const struct HsqsMetablock **metablock,
		const struct HsqsSuperblockContext *superblock, off_t offset);
HSQS_NO_UNUSED int hsqs_metablock_init(
		struct HsqsMetablockContext *extract,
		const struct HsqsSuperblockContext *superblock, off_t start_block);
HSQS_NO_UNUSED int hsqs_metablock_seek(
		struct HsqsMetablockContext *metablock, off_t index, off_t offset);
HSQS_NO_UNUSED int
hsqs_metablock_more(struct HsqsMetablockContext *metablock, const size_t size);
const uint8_t *
hsqs_metablock_data(const struct HsqsMetablockContext *metablock);
size_t hsqs_metablock_size(const struct HsqsMetablockContext *metablock);
int hsqs_metablock_cleanup(struct HsqsMetablockContext *metablock);

#endif /* end of include guard HSQS_EXTRACT_H */
