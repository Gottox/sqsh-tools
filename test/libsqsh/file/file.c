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
 * @file         inode.c
 */

#include "../common.h"
#include <testlib.h>

#include "../../libsqsh/src/utils/utils.h"
#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_file_private.h>

static void
load_file(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshFile file = {0};
	uint8_t payload[8192] = {
			SQSH_HEADER,
			/* inode */
			[INODE_TABLE_OFFSET + 15] = METABLOCK_HEADER(0, 128),
			0,
			0,
			0,
			INODE_HEADER(2, 0666, 0, 0, 4242, 1),
			INODE_BASIC_FILE(1024, 0xFFFFFFFF, 0, 1),
			UINT32_BYTES(42),

	};
	mk_stub(&archive, payload, sizeof(payload));

	uint64_t inode_ref = sqsh_address_ref_create(15, 3);
	rv = sqsh__file_init(&file, &archive, inode_ref);
	assert(rv == 0);

	assert(sqsh_file_type(&file) == SQSH_FILE_TYPE_FILE);
	assert(sqsh_file_permission(&file) == 0666);
	assert(sqsh_file_modified_time(&file) == 4242);
	assert(sqsh_file_blocks_start(&file) == 1024);
	assert(sqsh_file_has_fragment(&file) == false);

	assert(sqsh_file_block_count(&file) == 1);
	assert(sqsh_file_block_size(&file, 0) == 42);

	sqsh__file_cleanup(&file);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(load_file)
END_TESTS
