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
 * @file         walker.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_inode_private.h"
#include "../../include/sqsh_tree_private.h"
#include "../../lib/utils/utils.h"
#include "sqsh_inode.h"

static void
walker_symlink_open(void) {
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
			DIRECTORY_ENTRY(128, 2, 3, 3),
			's', 'r', 'c',
			DIRECTORY_ENTRY(256, 3, 2, 3),
			't', 'g', 't',
			[FRAGMENT_TABLE_OFFSET] = 0,
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshTreeWalker walker = {0};
	rv = sqsh__tree_walker_init(&walker, &archive);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "src", true);
	assert(rv == 0);

	sqsh__tree_walker_cleanup(&walker);
	sqsh__archive_cleanup(&archive);
}

DEFINE
TEST(walker_symlink_open);
DEFINE_END
