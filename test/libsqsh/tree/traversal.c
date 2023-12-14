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
#include <stdint.h>
#include <testlib.h>

#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_tree.h>
#include <sqsh_tree_private.h>

static void
test_recursive_directory(void) {
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
			DIRECTORY_ENTRY(0, 2, 1, 1),
			'r',
			[FRAGMENT_TABLE_OFFSET] = 0,
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshFile file = {0};
	rv = sqsh__file_init(&file, &archive, 0);
	assert(rv == 0);

	struct SqshTreeTraversal traversal = {0};
	rv = sqsh__tree_traversal_init(&traversal, &file);
	assert(rv == 0);

	for (int i = 0;; i++) {
		bool has_next = sqsh_tree_traversal_next(&traversal, &rv);
		if (i < 128) {
			assert(has_next);
			assert(rv == 0);
		} else {
			assert(!has_next);
			assert(rv == -SQSH_ERROR_DIRECTORY_RECURSION);
			break;
		}
	}

	sqsh__tree_traversal_cleanup(&traversal);
	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(test_recursive_directory)
END_TESTS
