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
 * @file         directory_iterator.c
 */

#include "../common.h"
#include <testlib.h>

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_directory.h"
#include "../../include/sqsh_directory_private.h"
#include "../../include/sqsh_inode_private.h"
#include "../../lib/utils/utils.h"

static void
iter_invalid_file_name_with_0(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 1024),
			INODE_HEADER(1, 0, 0, 0, 0, 1),
			INODE_BASIC_DIR(0, 1024, 0, 0),
			[DIRECTORY_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			DIRECTORY_HEADER(1, 0, 0),
			DIRECTORY_ENTRY(128, 2, 3, 6),
			'\0', 'H', 'e', 'l', 'l', 'o',
			[FRAGMENT_TABLE_OFFSET] = 0,
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInode inode = {0};
	rv = sqsh__inode_init(&inode, &archive, 0);
	assert(rv == 0);

	struct SqshDirectoryIterator iter = {0};
	rv = sqsh__directory_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(&iter);
	assert(rv == -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY);

	sqsh__directory_iterator_cleanup(&iter);
	sqsh__inode_cleanup(&inode);
	sqsh__archive_cleanup(&archive);
}

static void
iter_invalid_file_name_with_slash(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 1024),
			INODE_HEADER(1, 0, 0, 0, 0, 1),
			INODE_BASIC_DIR(0, 1024, 0, 0),
			[DIRECTORY_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			DIRECTORY_HEADER(1, 0, 0),
			DIRECTORY_ENTRY(128, 2, 3, 11),
			'/', 'e', 't', 'c', '/', 'p', 'a', 's', 's', 'w', 'd',
			[FRAGMENT_TABLE_OFFSET] = 0,
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInode inode = {0};
	rv = sqsh__inode_init(&inode, &archive, 0);
	assert(rv == 0);

	struct SqshDirectoryIterator iter = {0};
	rv = sqsh__directory_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(&iter);
	assert(rv == -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY);

	sqsh__directory_iterator_cleanup(&iter);
	sqsh__inode_cleanup(&inode);
	sqsh__archive_cleanup(&archive);
}

static void
iter_two_files(void) {
	int rv;
	struct SqshArchive archive = {0};
	uint8_t payload[] = {
			/* clang-format off */
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET] = METABLOCK_HEADER(0, 1024),
			INODE_HEADER(1, 0, 0, 0, 0, 1),
			INODE_BASIC_DIR(0, 1024, 0, 0),
			[INODE_TABLE_OFFSET+2+128] = 
			INODE_HEADER(3, 0, 0, 0, 0, 2),
			INODE_BASIC_SYMLINK(3),
			't', 'g', 't',
			[INODE_TABLE_OFFSET+2+256] =
			INODE_HEADER(2, 0, 0, 0, 0, 3),
			INODE_BASIC_FILE(0, 0xFFFFFFFF, 0, 0),
			[DIRECTORY_TABLE_OFFSET] = METABLOCK_HEADER(0, 128),
			DIRECTORY_HEADER(2, 0, 0),
			DIRECTORY_ENTRY(128, 2, 3, 1),
			'1',
			DIRECTORY_ENTRY(256, 3, 2, 1),
			'2',
			[FRAGMENT_TABLE_OFFSET] = 0,
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInode inode = {0};
	rv = sqsh__inode_init(&inode, &archive, 0);
	assert(rv == 0);

	struct SqshDirectoryIterator iter = {0};
	rv = sqsh__directory_iterator_init(&iter, &inode);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(&iter);
	assert(rv > 0);

	const char *name = sqsh_directory_iterator_name(&iter);
	size_t size = sqsh_directory_iterator_name_size(&iter);
	assert(size == 1);
	assert(name[0] == '1');

	rv = sqsh_directory_iterator_next(&iter);
	assert(rv > 0);

	name = sqsh_directory_iterator_name(&iter);
	size = sqsh_directory_iterator_name_size(&iter);
	assert(size == 1);
	assert(name[0] == '2');

	sqsh__directory_iterator_cleanup(&iter);
	sqsh__inode_cleanup(&inode);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(iter_two_files)
TEST(iter_invalid_file_name_with_slash)
TEST(iter_invalid_file_name_with_0)
END_TESTS
