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
 * @file         xattr_iterator.c
 */

#include "../common.h"
#include <testlib.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_file_private.h>
#include <sqsh_xattr.h>

static void
load_xattr(void) {
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
			INODE_HEADER(8, 0, 0, 0, 0, 1),
			INODE_EXT_DIR(0, 1024, 0, 0, 0, 0),
			[XATTR_TABLE_OFFSET] =
					XATTR_LOOKUP_HEADER(XATTR_TABLE_OFFSET + 128, 1),
			UINT64_BYTES(XATTR_TABLE_OFFSET + 128),
			[XATTR_TABLE_OFFSET + 128] = METABLOCK_HEADER(0, 128),
			XATTR_LOOKUP_ENTRY(
					128, 5, 3, 44),
			[XATTR_TABLE_OFFSET + 256] = METABLOCK_HEADER(0, 128),
			0xa1, 0xa1, 0xa1, 0xa1, 0xa1, /* offset bytes */
			XATTR_NAME_HEADER(0, 3),
			'a', 'b', 'c',
			XATTR_VALUE_HEADER(4),
			'd', 'e', 'f', 'g',
			XATTR_NAME_HEADER(1, 3),
			'h', 'i', 'j',
			XATTR_VALUE_HEADER(3),
			'k', 'l', 'm',
			XATTR_NAME_HEADER(2, 3),
			'n', 'o', 'p',
			XATTR_VALUE_HEADER(3),
			'q', 'r', 's',
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(SQSH_FILE_TYPE_DIRECTORY, (int)sqsh_file_type(&file));
	ASSERT_EQ(true, sqsh_file_is_extended(&file));

	struct SqshXattrIterator *iterator = sqsh_xattr_iterator_new(&file, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, iterator);

	bool has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(true, has_next);
	size_t size = sqsh_xattr_iterator_name_size(iterator);
	ASSERT_EQ((size_t)3, size);
	const char *name = sqsh_xattr_iterator_name(iterator);
	ASSERT_EQ(0, memcmp(name, "abc", size));
	size = sqsh_xattr_iterator_value_size(iterator);
	ASSERT_EQ((size_t)4, size);
	const char *value = sqsh_xattr_iterator_value(iterator);
	ASSERT_EQ(0, memcmp(value, "defg", size));
	const char *prefix = sqsh_xattr_iterator_prefix(iterator);
	ASSERT_EQ(0, strcmp(prefix, "user."));

	has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(true, has_next);
	size = sqsh_xattr_iterator_name_size(iterator);
	ASSERT_EQ((size_t)3, size);
	name = sqsh_xattr_iterator_name(iterator);
	ASSERT_EQ(0, memcmp(name, "hij", size));
	size = sqsh_xattr_iterator_value_size(iterator);
	ASSERT_EQ((size_t)3, size);
	value = sqsh_xattr_iterator_value(iterator);
	ASSERT_EQ(0, memcmp(value, "klm", size));
	prefix = sqsh_xattr_iterator_prefix(iterator);
	ASSERT_EQ(0, strcmp(prefix, "trusted."));

	has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(true, has_next);
	size = sqsh_xattr_iterator_name_size(iterator);
	ASSERT_EQ((size_t)3, size);
	name = sqsh_xattr_iterator_name(iterator);
	ASSERT_EQ(0, memcmp(name, "nop", size));
	size = sqsh_xattr_iterator_value_size(iterator);
	ASSERT_EQ((size_t)3, size);
	value = sqsh_xattr_iterator_value(iterator);
	ASSERT_EQ(0, memcmp(value, "qrs", size));
	prefix = sqsh_xattr_iterator_prefix(iterator);
	ASSERT_EQ(0, strcmp(prefix, "security."));

	has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(false, has_next);

	sqsh_xattr_iterator_free(iterator);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

static void
load_xattr_indirect(void) {
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
			INODE_HEADER(8, 0, 0, 0, 0, 1),
			INODE_EXT_DIR(0, 1024, 0, 0, 0, 0),
			[XATTR_TABLE_OFFSET] =
					XATTR_LOOKUP_HEADER(XATTR_TABLE_OFFSET + 128, 1),
			UINT64_BYTES(XATTR_TABLE_OFFSET + 128),
			[XATTR_TABLE_OFFSET + 128] = METABLOCK_HEADER(0, 128),
			XATTR_LOOKUP_ENTRY(
					128, 0, 1, 12),
			[XATTR_TABLE_OFFSET + 128 + 128] = METABLOCK_HEADER(0, 128),
			XATTR_NAME_HEADER(0 | 0x0100, 3),
			'a', 'b', 'c',
			XATTR_VALUE_HEADER(8),
			UINT64_BYTES((256 << 16) + 5),
			[XATTR_TABLE_OFFSET + 128 + 256] = METABLOCK_HEADER(0, 128),
			0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 
			XATTR_VALUE_HEADER(3),
			'1', '2', '3',
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(0, 0);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	ASSERT_EQ(0, rv);

	ASSERT_EQ(SQSH_FILE_TYPE_DIRECTORY, (int)sqsh_file_type(&file));
	ASSERT_EQ(true, sqsh_file_is_extended(&file));

	struct SqshXattrIterator *iterator = sqsh_xattr_iterator_new(&file, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, iterator);

	bool has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(true, has_next);
	size_t size = sqsh_xattr_iterator_name_size(iterator);
	ASSERT_EQ((size_t)3, size);
	const char *name = sqsh_xattr_iterator_name(iterator);
	ASSERT_EQ(0, memcmp(name, "abc", size));
	size = sqsh_xattr_iterator_value_size(iterator);
	ASSERT_EQ((size_t)3, size);
	const char *value = sqsh_xattr_iterator_value(iterator);
	ASSERT_EQ(0, memcmp(value, "123", size));

	has_next = sqsh_xattr_iterator_next(iterator, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(false, has_next);

	sqsh_xattr_iterator_free(iterator);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(load_xattr)
TEST(load_xattr_indirect)
END_TESTS
