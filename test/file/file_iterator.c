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
#include <testlib.h>

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data_private.h"
#include "../../lib/utils/utils.h"

static const size_t BLOCK_SIZE = 32768;
static const size_t ZERO_BLOCK_SIZE = 16384;

static void
load_segment_from_compressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* datablock */
			[1024] = ZLIB_ABCD,
			/* inode */
			[INODE_TABLE_OFFSET + 256] = METABLOCK_HEADER(0, 128), 0, 0, 0,
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(1024, 0xFFFFFFFF, 0, 4),
			DATA_BLOCK_REF(sizeof((uint8_t[]){ZLIB_ABCD}), 1),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(256, 3);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
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

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_two_segments_from_uncompressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(8192, 0xFFFFFFFF, 0, BLOCK_SIZE + 5),
			DATA_BLOCK_REF(1000, 0),
			DATA_BLOCK_REF(5, 0),
			/* datablocks */
			[8192 ... 8192 + 999] = 0xa1,
			[8192 + 1000] = 1, 2, 3, 4, 5
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == 1000);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	for (size_t i = 0; i < size; i++) {
		assert(data[i] == 0xa1);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == ZERO_BLOCK_SIZE);
	data = sqsh_file_iterator_data(&iter);
	for (size_t i = 0; i < ZERO_BLOCK_SIZE; i++) {
		assert(data[i] == 0);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == ZERO_BLOCK_SIZE - 1000);
	data = sqsh_file_iterator_data(&iter);
	for (size_t i = 0; i < ZERO_BLOCK_SIZE - 1000; i++) {
		assert(data[i] == 0);
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

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_segment_from_uncompressed_data_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* datablock */
			[512] = 1, 2, 3, 4, 5,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, 5),
			DATA_BLOCK_REF(5, 0),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
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

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_zero_padding(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, BLOCK_SIZE + 5),
			DATA_BLOCK_REF(0, 1),
			DATA_BLOCK_REF(0, 1),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == BLOCK_SIZE);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 5);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	assert(data[0] == 0);
	assert(data[1] == 0);
	assert(data[2] == 0);
	assert(data[3] == 0);
	assert(data[4] == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 0);
	data = sqsh_file_iterator_data(&iter);
	assert(data == NULL);

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_zero_big_padding(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, BLOCK_SIZE + 5),
			DATA_BLOCK_REF(0, 1),
			DATA_BLOCK_REF(0, 1),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == BLOCK_SIZE);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 5);
	const uint8_t *data = sqsh_file_iterator_data(&iter);
	assert(data[0] == 0);
	assert(data[1] == 0);
	assert(data[2] == 0);
	assert(data[3] == 0);
	assert(data[4] == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == 0);
	data = sqsh_file_iterator_data(&iter);
	assert(data == NULL);

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_zero_block(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, ZERO_BLOCK_SIZE),
			DATA_BLOCK_REF(0, 0),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == ZERO_BLOCK_SIZE);

	for (size_t i = 0; i < ZERO_BLOCK_SIZE; i++) {
		const uint8_t *data = sqsh_file_iterator_data(&iter);
		assert(data[i] == 0);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_two_zero_blocks(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			INODE_HEADER(2, 0, 0, 0, 0, 1),
			INODE_BASIC_FILE(512, 0xFFFFFFFF, 0, ZERO_BLOCK_SIZE * 2),
			DATA_BLOCK_REF(0, 0),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_has_fragment(&file) == false);

	struct SqshFileIterator iter = {0};
	rv = sqsh__file_iterator_init(&iter, &file);
	assert(rv == 0);

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size_t size = sqsh_file_iterator_size(&iter);
	assert(size == ZERO_BLOCK_SIZE);

	for (size_t i = 0; i < ZERO_BLOCK_SIZE; i++) {
		const uint8_t *data = sqsh_file_iterator_data(&iter);
		assert(data[i] == 0);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv > 0);

	size = sqsh_file_iterator_size(&iter);
	assert(size == ZERO_BLOCK_SIZE);

	for (size_t i = 0; i < ZERO_BLOCK_SIZE; i++) {
		const uint8_t *data = sqsh_file_iterator_data(&iter);
		assert(data[i] == 0);
	}

	rv = sqsh_file_iterator_next(&iter, 1);
	assert(rv == 0);

	sqsh__file_iterator_cleanup(&iter);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(load_two_segments_from_uncompressed_data_block)
TEST(load_segment_from_uncompressed_data_block)
TEST(load_segment_from_compressed_data_block)
TEST(load_zero_padding)
TEST(load_zero_big_padding)
TEST(load_zero_block)
NO_TEST(load_two_zero_blocks)
END_TESTS
