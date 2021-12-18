/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : integration
 * @created     : Monday Oct 11, 2021 21:43:12 CEST
 */

#include "../src/context/content_context.h"
#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/superblock_context.h"
#include "../src/context/xattr_table_context.h"
#include "../src/data/superblock.h"
#include "../src/error.h"
#include "../src/hsqs.h"
#include "../src/resolve_path.h"
#include "common.h"
#include "test.h"
#include <squashfs_image.h>
#include <stdint.h>

static void
hsqs_empty() {
	int rv;
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, NULL, 0);
	assert(rv == -HSQS_ERROR_SUPERBLOCK_TOO_SMALL);
}

static void
hsqs_ls() {
	int rv;
	char *name;
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryContext dir = {0};
	struct HsqsDirectoryIterator iter = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_inode_load_root(&inode, &hsqs);
	assert(rv == 0);

	rv = hsqs_directory_init(&dir, &inode);
	assert(rv == 0);

	rv = hsqs_directory_iterator_init(&iter, &dir);
	assert(rv == 0);

	rv = hsqs_directory_iterator_next(&iter);
	assert(rv > 0);
	rv = hsqs_directory_iterator_name_dup(&iter, &name);
	assert(rv == 1);
	assert(strcmp("a", name) == 0);
	free(name);

	rv = hsqs_directory_iterator_next(&iter);
	assert(rv >= 0);
	rv = hsqs_directory_iterator_name_dup(&iter, &name);
	assert(rv == 1);
	assert(strcmp("b", name) == 0);
	free(name);

	rv = hsqs_directory_iterator_next(&iter);
	// End of file list
	assert(rv == 0);

	rv = hsqs_directory_iterator_cleanup(&iter);
	assert(rv == 0);

	rv = hsqs_directory_cleanup(&dir);
	assert(rv == 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
hsqs_cat_fragment() {
	int rv;
	const uint8_t *data;
	size_t size;
	struct HsqsInodeContext inode = {0};
	struct HsqsFileContext file = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "a");
	assert(rv == 0);

	rv = hsqs_content_init(&file, &inode);
	assert(rv == 0);

	size = hsqs_inode_file_size(&inode);
	assert(size == 2);

	rv = hsqs_content_read(&file, size);
	assert(rv == 0);

	data = hsqs_content_data(&file);
	assert(memcmp(data, "a\n", size) == 0);

	rv = hsqs_content_cleanup(&file);
	assert(rv == 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
hsqs_cat_datablock_and_fragment() {
	int rv;
	const uint8_t *data;
	size_t size;
	struct HsqsInodeContext inode = {0};
	struct HsqsFileContext file = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "b");
	assert(rv == 0);

	rv = hsqs_content_init(&file, &inode);
	assert(rv == 0);

	size = hsqs_inode_file_size(&inode);
	assert(size == 1050000);

	rv = hsqs_content_read(&file, size);
	assert(rv == 0);
	assert(size == hsqs_content_size(&file));

	data = hsqs_content_data(&file);
	for (int i = 0; i < size; i++) {
		assert(data[i] == 'b');
	}

	rv = hsqs_content_cleanup(&file);
	assert(rv == 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
hsqs_cat_size_overflow() {
	int rv;
	const uint8_t *data;
	size_t size;
	struct HsqsInodeContext inode = {0};
	struct HsqsFileContext file = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "b");
	assert(rv == 0);

	rv = hsqs_content_init(&file, &inode);
	assert(rv == 0);
	size = hsqs_inode_file_size(&inode);
	assert(size == 1050000);

	rv = hsqs_content_read(&file, size + 4096);
	assert(rv != 0); // TODO: check for correct error code

	assert(hsqs_content_size(&file) == size);

	data = hsqs_content_data(&file);
	for (int i = 0; i < size; i++) {
		assert(data[i] == 'b');
	}

	rv = hsqs_content_cleanup(&file);
	assert(rv == 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
hsqs_test_uid_and_gid() {
	int rv;
	uint32_t uid, gid;
	struct HsqsInodeContext inode = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_inode_load_root(&inode, &hsqs);
	assert(rv == 0);

	uid = hsqs_inode_uid(&inode);
	assert(uid == 2020);
	gid = hsqs_inode_gid(&inode);
	assert(gid == 202020);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
hsqs_test_xattr() {
	const char *expected_value = "1234567891234567891234567890001234567890";
	int rv;
	char *name, *value;
	struct HsqsInodeContext inode = {0};
	struct HsqsInodeContext entry_inode = {0};
	struct HsqsDirectoryContext dir = {0};
	struct HsqsDirectoryIterator dir_iter = {0};
	struct HsqsXattrTableIterator xattr_iter = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = hsqs_inode_load_root(&inode, &hsqs);
	assert(rv == 0);

	rv = hsqs_inode_xattr_iterator(&inode, &xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_next(&xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_cleanup(&xattr_iter);
	assert(rv == 0);

	rv = hsqs_directory_init(&dir, &inode);
	assert(rv == 0);

	rv = hsqs_directory_iterator_init(&dir_iter, &dir);
	assert(rv == 0);

	rv = hsqs_directory_iterator_next(&dir_iter);
	assert(rv > 0);
	rv = hsqs_directory_iterator_name_dup(&dir_iter, &name);
	assert(rv == 1);
	assert(strcmp("a", name) == 0);
	free(name);
	rv = hsqs_directory_iterator_inode_load(&dir_iter, &entry_inode);
	assert(rv == 0);
	rv = hsqs_inode_xattr_iterator(&entry_inode, &xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_next(&xattr_iter);
	assert(rv > 0);
	assert(hsqs_xattr_table_iterator_is_indirect(&xattr_iter) == false);
	rv = hsqs_xattr_table_iterator_fullname_dup(&xattr_iter, &name);
	assert(rv == 8);
	assert(strcmp("user.foo", name) == 0);
	free(name);
	rv = hsqs_xattr_table_iterator_value_dup(&xattr_iter, &value);
	assert(rv == strlen(expected_value));
	assert(strcmp(expected_value, value) == 0);
	free(value);
	rv = hsqs_xattr_table_iterator_next(&xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_cleanup(&xattr_iter);
	assert(rv == 0);
	rv = hsqs_inode_cleanup(&entry_inode);
	assert(rv == 0);

	rv = hsqs_directory_iterator_next(&dir_iter);
	assert(rv >= 0);
	rv = hsqs_directory_iterator_name_dup(&dir_iter, &name);
	assert(rv == 1);
	assert(strcmp("b", name) == 0);
	free(name);
	rv = hsqs_directory_iterator_inode_load(&dir_iter, &entry_inode);
	assert(rv == 0);
	rv = hsqs_inode_xattr_iterator(&entry_inode, &xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_next(&xattr_iter);
	assert(rv > 0);
	assert(hsqs_xattr_table_iterator_is_indirect(&xattr_iter) == true);
	rv = hsqs_xattr_table_iterator_fullname_dup(&xattr_iter, &name);
	assert(rv == 8);
	assert(strcmp("user.bar", name) == 0);
	free(name);
	rv = hsqs_xattr_table_iterator_value_dup(&xattr_iter, &value);
	assert(rv == strlen(expected_value));
	assert(strcmp(expected_value, value) == 0);
	free(value);
	rv = hsqs_xattr_table_iterator_next(&xattr_iter);
	assert(rv == 0);
	rv = hsqs_xattr_table_iterator_cleanup(&xattr_iter);
	assert(rv == 0);
	rv = hsqs_inode_cleanup(&entry_inode);
	assert(rv == 0);

	rv = hsqs_directory_iterator_next(&dir_iter);
	// End of file list
	assert(rv == 0);

	rv = hsqs_directory_iterator_cleanup(&dir_iter);
	assert(rv == 0);

	rv = hsqs_directory_cleanup(&dir);
	assert(rv == 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
fuzz_crash_1() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x3,  0x0,  0x0,  0x0,  0x96, 0x97, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x3e, 0x1,  0x0,  0x0,  0x0,  0x0,  0x3,  0x0,
			0x0,  0x64, 0x1,  0x1d, 0x0,  0x0,  0x96, 0x97, 0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x3e, 0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x32, 0x62, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x32, 0x0,  0x0,
			0x0,  0x0,  0x0,  0x60, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62,
			0x1,  0x0,  0x62, 0x62, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x36,
			0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62,
			0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62,
			0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62,
			0x62, 0x62, 0x62, 0x62, 0x62, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x60, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x1,  0x0,
			0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0xfa, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0x36, 0x62, 0x62, 0x62, 0x62, 0x62,
			0x62, 0x62, 0x29, 0x62, 0x62, 0x62, 0x62, 0xff, 0xff, 0x62, 0x62};

	struct HsqsInodeContext inode = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "");
	assert(rv < 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
fuzz_crash_2() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x23, 0x0,  0x0,  0x0,  0x96, 0x97, 0x68,
			0x61, 0x1,  0x0,  0x2,  0x0,  0x1,  0x0,  0x10, 0x0,  0x1,  0x0,
			0x11, 0x0,  0xcb, 0x1,  0x1,  0x0,  0x4,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x7,  0x0,  0x0,  0x0,  0x0,  0x0,  0x64, 0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x62, 0x62, 0x62, 0x62,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x60, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x62, 0x62,
			0x62, 0x62, 0x62, 0x11, 0x0,  0xcb, 0x1,  0x1,  0x0,  0x4,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x60, 0x0,  0x0,  0x0,  0x62,
			0x62, 0x62, 0x0,  0x2,
	};

	struct HsqsInodeContext inode = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "");
	assert(rv < 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
fuzz_crash_3() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x23, 0x0, 0x0,  0x0,  0x96, 0x97, 0x68,
			0x61, 0x1,  0x0,  0x2,  0x0,  0x1, 0x1,  0x10, 0x0,  0x5,  0x0,
			0x11, 0x0,  0xcb, 0x1,  0x1,  0x0, 0x4,  0x0,  0x0,  0x0,  0x0,
			0xb9, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x64, 0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x76, 0x0, 0x0,  0x0,  0x62, 0x62, 0x62,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x40, 0x0,  0x0,  0x60, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0,  0x0,  0x1,  0x1d,
			0x73, 0x71, 0x73, 0x23, 0x0,  0x0, 0x0,  0x96, 0x97, 0x68, 0x61,
			0x1,  0x0,  0x2,  0x0,  0x1,  0x1, 0x0,  0x0,  0x2,  0x0,  0x11,
			0x0,  0xcb, 0x74, 0x71, 0x0,  0x0, 0x74, 0x71, 0x0,  0x0,  0x68,
			0x61, 0x1,  0x0,  0x0,  0x2,  0x2,
	};

	struct HsqsInodeContext inode = {0};
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);

	rv = hsqs_resolve_path(&inode, &hsqs, "");
	assert(rv < 0);

	rv = hsqs_inode_cleanup(&inode);
	assert(rv == 0);

	rv = hsqs_cleanup(&hsqs);
	assert(rv == 0);
}

static void
fuzz_crash_4() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0xa,  0xf8, 0x0,  0x0,  0x0,  0x0,  0xb1,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x30, 0x0,  0x8c, 0x0,  0x0,  0x0,
			0x0,  0x0,  0x6,  0x0,  0x0,  0x0,  0x0,  0x1,  0x68, 0x73, 0xf4,
			0xa,  0x41, 0x0,  0x0,  0x0,  0x0,  0x0,  0xf1, 0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0xf1, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x84, 0x0,  0x0,  0x0,  0xad, 0x0,  0x0,  0x0,  0x71, 0x1,
			0x0,  0x0,  0x1,  0x0,  0x0,  0x0,  0x62, 0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x2,  0x1,  0x0,  0x0,  0x68, 0x73,
			0x71, 0x73, 0xa,  0x0,  0xb1, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x1,  0x68, 0x73, 0x71, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf,
			0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf,
			0xaf, 0x73, 0xa,  0xff, 0xff, 0x0,  0x23, 0x0,  0x62, 0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x2,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x1,  0xb,  0x0,  0x2,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x68, 0x0,  0x0,  0x31, 0x0,  0x0,  0x2,  0x73,
			0x1d, 0x1d, 0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x62,
			0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x14, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
			0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x71, 0x3b, 0x3b, 0x11, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0xa,  0x0,  0x0,  0x0,  0x0,  0x0,
			0xb1, 0x0,  0x0,  0x0,  0x0,  0x0,  0x30, 0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
	};

	struct HsqsTableContext *id_table = NULL;
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);
	rv = hsqs_id_table(&hsqs, &id_table);
	assert(rv == -HSQS_ERROR_SIZE_MISSMATCH);
	hsqs_cleanup(&hsqs);
}

static void
fuzz_crash_5() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb1,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x8c, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0xfb, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x00, 0xf1, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x00,
			0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00,
			0x00, 0x68, 0x73, 0x71, 0x73, 0x0a, 0x00, 0xb1, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x01, 0x68, 0x73, 0x71, 0xaf, 0xaf, 0xaf,
			0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf,
			0xaf, 0xaf, 0xaf, 0xaf, 0x73, 0x0a, 0xff, 0xff, 0x00, 0x23, 0x00,
			0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0b, 0x00, 0x02, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
			0x73, 0x1d, 0x1d, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x62, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x14, 0x1d, 0x1d, 0x1d,
			0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x71, 0x3b, 0x3b,
			0x11, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfd, 0x00, 0x00, 0x00, 0x00,
			0x68, 0x73, 0x71, 0x73, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb1,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x02, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00,
	};

	struct HsqsTableContext *id_table = NULL;
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);
	rv = hsqs_id_table(&hsqs, &id_table);
	assert(rv == -HSQS_ERROR_SIZE_MISSMATCH);
	hsqs_cleanup(&hsqs);
}

static void
fuzz_crash_6() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x0,  0x0, 0x0, 0x0, 0x80, 0x0,  0x0, 0x1,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0x0,
			0x0,  0x0,  0x0,  0x69, 0xfb, 0x0, 0x0, 0x0, 0x0,  0x10, 0x0, 0x0,
			0x0,  0x0,  0xf7, 0x0,  0x60, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0x0,
			0x0,  0x1,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0, 0xa,
	};

	struct HsqsTableContext *id_table = NULL;
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);
	rv = hsqs_id_table(&hsqs, &id_table);
	assert(rv == -HSQS_ERROR_SIZE_MISSMATCH);
	hsqs_cleanup(&hsqs);
}

static void
fuzz_crash_7() {
	int rv;
	static const uint8_t input[] = {
			0x68, 0x73, 0x71, 0x73, 0x97, 0x97, 0x97, 0x97, 0x97, 0x97, 0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x7e, 0xa6, 0xa6, 0xa6,
			0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
			0x0,  0x97, 0x97, 0x97, 0x97, 0x74, 0x97, 0x97,
	};

	struct HsqsTableContext *id_table = NULL;
	struct Hsqs hsqs = {0};
	rv = hsqs_init(&hsqs, input, sizeof(input));
	assert(rv == 0);
	rv = hsqs_id_table(&hsqs, &id_table);
	assert(rv == -HSQS_ERROR_SIZE_MISSMATCH);
	hsqs_cleanup(&hsqs);
}
DEFINE
TEST(hsqs_empty);
TEST(hsqs_ls);
TEST(hsqs_cat_fragment);
TEST(hsqs_cat_datablock_and_fragment);
TEST(hsqs_cat_size_overflow);
TEST(hsqs_test_uid_and_gid);
TEST(hsqs_test_xattr);
TEST_OFF(fuzz_crash_1); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_2); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_3); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_4);
TEST_OFF(fuzz_crash_5);
TEST_OFF(fuzz_crash_6);
TEST_OFF(fuzz_crash_7);
DEFINE_END
