/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2023, Enno Boland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         file_iterator.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_inode_private.h"
#include "../../lib/utils.h"

static void
load_segment_from_compressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInodeContext inode = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128), 0, 0, 0,
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 4),
			DATA_BLOCK_REF(sizeof((uint8_t[]){ZLIB_ABCD}), 1),
			/* datablock */
			[512] = ZLIB_ABCD
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 3);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == 4);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	assert(data[0] == 'a');
	assert(data[1] == 'b');
	assert(data[2] == 'c');
	assert(data[3] == 'd');

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 0);
	data = sqsh_file_iterator_data(&iter);
	assert(data == NULL);
}

static void
load_two_segments_from_uncompressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInodeContext inode = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 4096 + 5),
			DATA_BLOCK_REF(4096, 0),
			DATA_BLOCK_REF(5, 0),
			/* datablocks */
			[512 ... 512 + 4095] = 0xa1,
			[512 + 4096] = 1, 2, 3, 4, 5
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 0);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == 4096);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	for (size_t i = 0; i < size; i++) {
		assert(data[i] == 0xa1);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 5);
	data = sqsh_file_iterator_data(&iter);
	assert(data[0] == 1);
	assert(data[1] == 2);
	assert(data[2] == 3);
	assert(data[3] == 4);
	assert(data[4] == 5);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 0);
	data = sqsh_file_iterator_data(&iter);
	assert(data == NULL);
}

static void
load_segment_from_uncompressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInodeContext inode = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 5),
			DATA_BLOCK_REF(5, 0),
			/* datablock */
			[512] = 1, 2, 3, 4, 5
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 0);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == 5);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	assert(data[0] == 1);
	assert(data[1] == 2);
	assert(data[2] == 3);
	assert(data[3] == 4);
	assert(data[4] == 5);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 0);
	data = sqsh_file_iterator_data(&iter);
	assert(data == NULL);
}

DEFINE
TEST(load_two_segments_from_uncompressed_data_block);
TEST(load_segment_from_uncompressed_data_block);
TEST(load_segment_from_compressed_data_block);
DEFINE_END
