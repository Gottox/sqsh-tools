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
 * @file         inode_map.c
 */

#include "../common.h"
#include <utest.h>

#include <sqsh_archive_private.h>

UTEST(inode_map, insert_inode_ref) {
	int rv = 0;
	uint64_t inode_ref = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 1, 4242);
	ASSERT_EQ(0, rv);

	inode_ref = sqsh_inode_map_get2(&map, 1, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ((uint64_t)4242, inode_ref);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST(inode_map, insert_invalid_inode) {
	int rv = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 0, 4242);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);

	rv = sqsh_inode_map_set(&map, 1000, 4242);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST(inode_map, insert_invalid_inode_ref) {
	int rv = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 1, UINT64_MAX);
	ASSERT_EQ(-SQSH_ERROR_INVALID_ARGUMENT, rv);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST(inode_map, get_invalid_inode) {
	int rv = 0;
	uint64_t inode_ref = 4242;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	inode_ref = sqsh_inode_map_get2(&map, 424242, &rv);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);
	ASSERT_EQ((uint64_t)0, inode_ref);

	inode_ref = sqsh_inode_map_get2(&map, 0, &rv);
	ASSERT_EQ(-SQSH_ERROR_OUT_OF_BOUNDS, rv);
	ASSERT_EQ((uint64_t)0, inode_ref);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST(inode_map, get_unknown_inode_ref) {
	int rv = 0;
	uint64_t inode_ref = 4242;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	inode_ref = sqsh_inode_map_get2(&map, 1, &rv);
	ASSERT_EQ(-SQSH_ERROR_NO_SUCH_ELEMENT, rv);
	ASSERT_EQ((uint64_t)0, inode_ref);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST(inode_map, insert_inconsistent_mapping) {
	int rv = 0;
	uint8_t payload[8192] = {
			SQSH_HEADER,
	};
	struct SqshArchive archive = {0};
	mk_stub(&archive, payload, sizeof(payload));

	struct SqshInodeMap map = {0};

	rv = sqsh__inode_map_init(&map, &archive);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 1, 4242);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 1, 4242);
	ASSERT_EQ(0, rv);

	rv = sqsh_inode_map_set(&map, 1, 2424);
	ASSERT_EQ(-SQSH_ERROR_INODE_MAP_IS_INCONSISTENT, rv);

	sqsh__inode_map_cleanup(&map);
	sqsh__archive_cleanup(&archive);
}

UTEST_MAIN()
