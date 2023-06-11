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
#include "../../include/sqsh_inode_private.h"
#include "../../include/sqsh_xattr.h"
#include "../../lib/utils/utils.h"
#include "sqsh_inode.h"

static void
load_xattr(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInode inode = {0};
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
					128, 5, 3, 0 /* TODO: this value is ignored. See #28 */),
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
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_DIRECTORY);
	assert(sqsh_inode_is_extended(&inode) == true);

	struct SqshXattrIterator *iterator = sqsh_xattr_iterator_new(&inode, &rv);
	assert(rv == 0);
	assert(iterator != NULL);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv > 0);
	size_t size = sqsh_xattr_iterator_name_size(iterator);
	assert(size == 3);
	const char *name = sqsh_xattr_iterator_name(iterator);
	assert(memcmp(name, "abc", size) == 0);
	size = sqsh_xattr_iterator_value_size(iterator);
	assert(size == 4);
	const char *value = sqsh_xattr_iterator_value(iterator);
	assert(memcmp(value, "defg", size) == 0);
	const char *prefix = sqsh_xattr_iterator_prefix(iterator);
	assert(strcmp(prefix, "user.") == 0);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv > 0);
	size = sqsh_xattr_iterator_name_size(iterator);
	assert(size == 3);
	name = sqsh_xattr_iterator_name(iterator);
	assert(memcmp(name, "hij", size) == 0);
	size = sqsh_xattr_iterator_value_size(iterator);
	assert(size == 3);
	value = sqsh_xattr_iterator_value(iterator);
	assert(memcmp(value, "klm", size) == 0);
	prefix = sqsh_xattr_iterator_prefix(iterator);
	assert(strcmp(prefix, "trusted.") == 0);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv > 0);
	size = sqsh_xattr_iterator_name_size(iterator);
	assert(size == 3);
	name = sqsh_xattr_iterator_name(iterator);
	assert(memcmp(name, "nop", size) == 0);
	size = sqsh_xattr_iterator_value_size(iterator);
	assert(size == 3);
	value = sqsh_xattr_iterator_value(iterator);
	assert(memcmp(value, "qrs", size) == 0);
	prefix = sqsh_xattr_iterator_prefix(iterator);
	assert(strcmp(prefix, "security.") == 0);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv == 0);

	sqsh_xattr_iterator_free(iterator);
}

static void
load_xattr_indirect(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshInode inode = {0};
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
					128, 0, 1, 0 /* TODO: this value is ignored. See #28 */),
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
	rv = sqsh__inode_init(&inode, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_inode_type(&inode) == SQSH_INODE_TYPE_DIRECTORY);
	assert(sqsh_inode_is_extended(&inode) == true);

	struct SqshXattrIterator *iterator = sqsh_xattr_iterator_new(&inode, &rv);
	assert(rv == 0);
	assert(iterator != NULL);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv > 0);
	size_t size = sqsh_xattr_iterator_name_size(iterator);
	assert(size == 3);
	const char *name = sqsh_xattr_iterator_name(iterator);
	assert(memcmp(name, "abc", size) == 0);
	size = sqsh_xattr_iterator_value_size(iterator);
	assert(size == 3);
	const char *value = sqsh_xattr_iterator_value(iterator);
	assert(memcmp(value, "123", size) == 0);

	rv = sqsh_xattr_iterator_next(iterator);
	assert(rv == 0);

	sqsh_xattr_iterator_free(iterator);
}

DEFINE
TEST(load_xattr);
TEST(load_xattr_indirect);
DEFINE_END
