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
 * @file         file_reader.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data.h"
#include "../../include/sqsh_inode_private.h"
#include "../../lib/utils.h"
#include "sqsh_file.h"

static void
load_file_from_compressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInode inode = {0};
	uint8_t payload[4096] = {
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128), 0, 0, 0,
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 4),
			DATA_BLOCK_REF(sizeof((uint8_t[]){ZLIB_ABCD}), 1),
			/* datablock */
			[512] = ZLIB_ABCD};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 3);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileReader reader = {0};
	rv = sqsh__file_reader_init(&reader, &inode);
	assert(rv == 0);

	rv = sqsh_file_reader_advance(&reader, 0, 4);
	assert(rv == 0);

	size_t size = sqsh_file_reader_size(&reader);
	assert(size == 4);
	const uint8_t *data = sqsh_file_reader_data(&reader);
	assert(data[0] == 'a');
	assert(data[1] == 'b');
	assert(data[2] == 'c');
	assert(data[3] == 'd');
}

static void
load_file_from_compressed_data_block_with_offset(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInode inode = {0};
	uint8_t payload[4096] = {
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128), 0, 0, 0,
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 4),
			DATA_BLOCK_REF(sizeof((uint8_t[]){ZLIB_ABCD}), 1),
			/* datablock */
			[512] = ZLIB_ABCD};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 3);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileReader reader = {0};
	rv = sqsh__file_reader_init(&reader, &inode);
	assert(rv == 0);

	rv = sqsh_file_reader_advance(&reader, 1, 2);
	assert(rv == 0);

	size_t size = sqsh_file_reader_size(&reader);
	assert(size == 2);
	const uint8_t *data = sqsh_file_reader_data(&reader);
	assert(data[0] == 'b');
	assert(data[1] == 'c');
}

static void
load_file_from_uncompressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInode inode = {0};
	uint8_t payload[4096] = {
			SQSH_HEADER,
			/* inode */
			[256] = METABLOCK_HEADER(0, 128), 0, 0, 0,
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 5), DATA_BLOCK_REF(5, 0),
			/* datablock */
			[512] = 1, 2, 3, 4, 5};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 3);
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_FILE);
	assert(sqsh_inode_file_has_fragment(&inode) == false);

	struct SqshFileReader reader = {0};
	rv = sqsh__file_reader_init(&reader, &inode);
	assert(rv == 0);

	rv = sqsh_file_reader_advance(&reader, 0, 5);
	assert(rv == 0);

	size_t size = sqsh_file_reader_size(&reader);
	assert(size == 5);
	const uint8_t *data = sqsh_file_reader_data(&reader);
	assert(data[0] == 1);
	assert(data[1] == 2);
	assert(data[2] == 3);
	assert(data[3] == 4);
	assert(data[4] == 5);
}

DEFINE
TEST(load_file_from_uncompressed_data_block);
TEST(load_file_from_compressed_data_block);
TEST(load_file_from_compressed_data_block_with_offset);
DEFINE_END
