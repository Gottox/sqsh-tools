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
 * @file         compression_options.c
 */

#include "../common.h"
#include <testlib.h>

#include "../../include/sqsh_archive_private.h"
#include "../../lib/utils/utils.h"

static void
insert_inode_ref(void) {
	int rv = 0;
	uint64_t inode_ref = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh_inode_map_init(&map, &archive);
	assert(rv == 0);

	rv = sqsh_inode_map_set(&map, 1, 4242);
	assert(rv == 0);

	inode_ref = sqsh_inode_map_get2(&map, 1, &rv);
	assert(rv == 0);
	assert(inode_ref == 4242);

	sqsh_inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

static void
insert_invalid_inode(void) {
	int rv = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh_inode_map_init(&map, &archive);
	assert(rv == 0);

	rv = sqsh_inode_map_set(&map, 0, 4242);
	assert(rv == -SQSH_ERROR_OUT_OF_BOUNDS);

	rv = sqsh_inode_map_set(&map, 1000, 4242);
	assert(rv == -SQSH_ERROR_OUT_OF_BOUNDS);

	sqsh_inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

static void
insert_invalid_inode_ref(void) {
	int rv = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh_inode_map_init(&map, &archive);
	assert(rv == 0);

	rv = sqsh_inode_map_set(&map, 1, UINT64_MAX);
	assert(rv == -SQSH_ERROR_INVALID_ARGUMENT);

	sqsh_inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

DECLARE_TESTS
TEST(insert_inode_ref)
TEST(insert_invalid_inode)
TEST(insert_invalid_inode_ref)
END_TESTS
