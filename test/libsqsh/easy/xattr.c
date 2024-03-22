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
 * @file         xattr.c
 */

#include "../common.h"
#include <utest.h>

#include <sqsh_archive_private.h>
#include <sqsh_easy.h>

UTEST(ease_xattr, load_easy_xattr) {
	int rv;
	struct SqshArchive archive = {0};
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

	char **xattr_keys = sqsh_easy_xattr_keys(&archive, "/", &rv);
	char *value;
	ASSERT_EQ(0, rv);
	ASSERT_NE(NULL, xattr_keys);
	ASSERT_EQ(0, strcmp(xattr_keys[0], "user.abc"));
	value = sqsh_easy_xattr_get(&archive, "/", "user.abc", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(0, strcmp(value, "defg"));
	free(value);

	ASSERT_EQ(0, strcmp(xattr_keys[1], "trusted.hij"));
	value = sqsh_easy_xattr_get(&archive, "/", "trusted.hij", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(0, strcmp(value, "klm"));
	free(value);

	ASSERT_EQ(0, strcmp(xattr_keys[2], "security.nop"));
	value = sqsh_easy_xattr_get(&archive, "/", "security.nop", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(0, strcmp(value, "qrs"));
	free(value);

	ASSERT_EQ(NULL, xattr_keys[3]);
	value = sqsh_easy_xattr_get(&archive, "/", "non-existing", &rv);
	ASSERT_EQ(-SQSH_ERROR_NO_SUCH_XATTR, rv);
	ASSERT_EQ(NULL, value);

	free(xattr_keys);
	sqsh__archive_cleanup(&archive);
}

UTEST_MAIN()
