/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2024, Enno Boland
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
 * @file         export_table.c
 */

#include "../common.h"
#include <testlib.h>

#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_table_private.h>

static void
export_table__resolves_inodes(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshExportTable table = {0};
	uint64_t inode_ref = 0;
	uint8_t payload[8192] = {
			/* clang-format off */
                        SQSH_HEADER,
                        [EXPORT_TABLE_OFFSET] = UINT64_BYTES(EXPORT_TABLE_OFFSET + 8),
                        [EXPORT_TABLE_OFFSET + 8] = METABLOCK_HEADER(0, 800),
                        [EXPORT_TABLE_OFFSET + 8 + sizeof(struct SqshDataMetablock)] = UINT64_BYTES(0x10),
                        [EXPORT_TABLE_OFFSET + 16 + sizeof(struct SqshDataMetablock)] = UINT64_BYTES(0x20),
                        [EXPORT_TABLE_OFFSET + 24 + sizeof(struct SqshDataMetablock)] = UINT64_BYTES(0x30),
			/* clang-format on */
	};
	mk_stub(&archive, payload, sizeof(payload));

	rv = sqsh__export_table_init(&table, &archive);
	ASSERT_EQ(0, rv);

	rv = sqsh_export_table_resolve_inode(&table, 0, &inode_ref);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((uint64_t)0x10, inode_ref);

	rv = sqsh_export_table_resolve_inode2(&table, 1, &inode_ref);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((uint64_t)0x10, inode_ref);

	rv = sqsh_export_table_resolve_inode2(&table, 3, &inode_ref);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((uint64_t)0x30, inode_ref);

	rv = sqsh__export_table_cleanup(&table);
	ASSERT_EQ(0, rv);

	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(export_table__resolves_inodes)
END_TESTS
