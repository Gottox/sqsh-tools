/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @author       Enno Boland (mail@eboland.de)
 * @file         integration.c
 */

#include "common.h"
#include "sqsh_tree.h"
#include "test.h"
#include <pthread.h>
#include <sqsh_archive_private.h>
#include <sqsh_directory_private.h>
#include <sqsh_file_private.h>
#include <sqsh_inode_private.h>
#include <sqsh_tree_private.h>
#include <squashfs_image.h>

static void
sqsh_empty(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = DEFAULT_CONFIG(0);
	rv = sqsh__archive_init(&sqsh, NULL, &config);
	assert(rv == -SQSH_ERROR_SUPERBLOCK_TOO_SMALL);
}

static void
sqsh_get_nonexistant(void) {
	int rv;
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};

	const struct SqshConfig config = DEFAULT_CONFIG(test_squashfs_image_len);
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "/nonexistant");
	assert(rv < 0);

	rv = sqsh__tree_walker_cleanup(&walker);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
tree_walker(void) {
	int rv;
	struct SqshTreeWalker walker = {0};
	struct SqshArchive sqsh = {0};
	struct SqshInode *inode;
	const struct SqshConfig config = DEFAULT_CONFIG(test_squashfs_image_len);
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "/large_dir");
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "999");
	assert(rv == 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv == 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__tree_walker_cleanup(&walker);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_ls(void) {
	int rv;
	char *name;
	struct SqshInode inode = {0};
	struct SqshDirectoryIterator *iter = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshSuperblock *superblock;
	const struct SqshConfig config = DEFAULT_CONFIG(test_squashfs_image_len);
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	superblock = sqsh_archive_superblock(&sqsh);
	rv = sqsh__inode_init(
			&inode, &sqsh, sqsh_superblock_inode_root_ref(superblock));
	assert(rv == 0);

	iter = sqsh_directory_iterator_new(&inode, &rv);
	assert(iter != NULL);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(iter);
	assert(rv > 0);
	name = sqsh_directory_iterator_name_dup(iter);
	assert(name != NULL);
	assert(strcmp("a", name) == 0);
	free(name);

	rv = sqsh_directory_iterator_next(iter);
	assert(rv >= 0);
	name = sqsh_directory_iterator_name_dup(iter);
	assert(name != NULL);
	assert(strcmp("b", name) == 0);
	free(name);

	rv = sqsh_directory_iterator_next(iter);
	assert(rv >= 0);
	name = sqsh_directory_iterator_name_dup(iter);
	assert(name != NULL);
	assert(strcmp("large_dir", name) == 0);
	free(name);

	rv = sqsh_directory_iterator_next(iter);
	// End of file list
	assert(rv == 0);

	rv = sqsh_directory_iterator_free(iter);
	assert(rv == 0);

	rv = sqsh__inode_cleanup(&inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_cat_fragment(void) {
	int rv;
	const uint8_t *data;
	size_t size;
	struct SqshInode *inode = NULL;
	struct SqshFileReader reader = {0};
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = DEFAULT_CONFIG(test_squashfs_image_len);
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "a");
	assert(rv == 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv == 0);

	rv = sqsh__file_reader_init(&reader, inode);
	assert(rv == 0);

	size = sqsh_inode_file_size(inode);
	assert(size == 2);

	rv = sqsh_file_reader_advance(&reader, 0, size);
	assert(rv == 0);

	data = sqsh_file_reader_data(&reader);
	assert(memcmp(data, "a\n", size) == 0);

	rv = sqsh__file_reader_cleanup(&reader);
	assert(rv == 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__tree_walker_cleanup(&walker);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_cat_datablock_and_fragment(void) {
	int rv;
	const uint8_t *data;
	size_t size;
	struct SqshInode *inode = NULL;
	struct SqshFileReader reader = {0};
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = test_squashfs_image_len,
	};
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "b");
	assert(rv == 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv == 0);

	rv = sqsh__file_reader_init(&reader, inode);
	assert(rv == 0);

	size = sqsh_inode_file_size(inode);
	assert(size == 1050000);

	rv = sqsh_file_reader_advance(&reader, 0, size);
	assert(rv == 0);
	assert(size == sqsh_file_reader_size(&reader));

	data = sqsh_file_reader_data(&reader);
	for (sqsh_index_t i = 0; i < size; i++) {
		assert(data[i] == 'b');
	}

	rv = sqsh__file_reader_cleanup(&reader);
	assert(rv == 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__tree_walker_cleanup(&walker);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_cat_size_overflow(void) {
	int rv;
	size_t size;
	struct SqshInode *inode = NULL;
	struct SqshFileReader reader = {0};
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = test_squashfs_image_len,
	};
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "b");
	assert(rv == 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv == 0);

	rv = sqsh__file_reader_init(&reader, inode);
	assert(rv == 0);
	size = sqsh_inode_file_size(inode);
	assert(size == 1050000);

	rv = sqsh_file_reader_advance(&reader, 0, size + 4096);
	assert(rv != 0); // TODO: check for correct error code

	rv = sqsh__file_reader_cleanup(&reader);
	assert(rv == 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__tree_walker_cleanup(&walker);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_test_uid_and_gid(void) {
	int rv;
	uint32_t uid, gid;
	struct SqshInode inode = {0};
	struct SqshArchive sqsh = {0};
	const struct SqshSuperblock *superblock;
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = test_squashfs_image_len,
	};
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	superblock = sqsh_archive_superblock(&sqsh);
	rv = sqsh__inode_init(
			&inode, &sqsh, sqsh_superblock_inode_root_ref(superblock));
	assert(rv == 0);

	uid = sqsh_inode_uid(&inode);
	assert(uid == 2020);
	gid = sqsh_inode_gid(&inode);
	assert(gid == 202020);

	rv = sqsh__inode_cleanup(&inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_test_extended_dir(void) {
	int rv;
	struct SqshInode *inode = NULL;
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = test_squashfs_image_len,
	};
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "/large_dir/999");
	assert(rv == 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__tree_walker_cleanup(&walker);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
sqsh_test_xattr(void) {
	const char *expected_value = "1234567891234567891234567890001234567890";
	int rv;
	char *name, *value;
	struct SqshInode inode = {0};
	struct SqshInode *entry_inode = NULL;
	struct SqshDirectoryIterator *dir_iter = NULL;
	struct SqshXattrIterator *xattr_iter = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshSuperblock *superblock;
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = test_squashfs_image_len,
	};
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	superblock = sqsh_archive_superblock(&sqsh);
	rv = sqsh__inode_init(
			&inode, &sqsh, sqsh_superblock_inode_root_ref(superblock));
	assert(rv == 0);

	xattr_iter = sqsh_xattr_iterator_new(&inode, &rv);
	assert(xattr_iter != NULL);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_next(xattr_iter);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_free(xattr_iter);
	assert(rv == 0);

	dir_iter = sqsh_directory_iterator_new(&inode, &rv);
	assert(dir_iter != NULL);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(dir_iter);
	assert(rv > 0);
	name = sqsh_directory_iterator_name_dup(dir_iter);
	assert(name != NULL);
	assert(strcmp("a", name) == 0);
	free(name);
	entry_inode = sqsh_directory_iterator_inode_load(dir_iter, &rv);
	assert(rv == 0);
	xattr_iter = sqsh_xattr_iterator_new(entry_inode, &rv);
	assert(xattr_iter != NULL);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_next(xattr_iter);
	assert(rv > 0);
	assert(sqsh_xattr_iterator_is_indirect(xattr_iter) == false);
	name = sqsh_xattr_iterator_fullname_dup(xattr_iter);
	assert(name != NULL);
	assert(strcmp("user.foo", name) == 0);
	free(name);
	value = sqsh_xattr_iterator_value_dup(xattr_iter);
	assert(value != NULL);
	assert(strcmp(expected_value, value) == 0);
	free(value);
	rv = sqsh_xattr_iterator_next(xattr_iter);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_free(xattr_iter);
	assert(rv == 0);
	rv = sqsh_inode_free(entry_inode);
	assert(rv == 0);

	rv = sqsh_directory_iterator_next(dir_iter);
	assert(rv >= 0);
	name = sqsh_directory_iterator_name_dup(dir_iter);
	assert(name != NULL);
	assert(strcmp("b", name) == 0);
	free(name);
	entry_inode = sqsh_directory_iterator_inode_load(dir_iter, &rv);
	assert(rv == 0);
	xattr_iter = sqsh_xattr_iterator_new(entry_inode, &rv);
	assert(xattr_iter != NULL);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_next(xattr_iter);
	assert(rv > 0);
	assert(sqsh_xattr_iterator_is_indirect(xattr_iter) == true);
	name = sqsh_xattr_iterator_fullname_dup(xattr_iter);
	assert(name != NULL);
	assert(strcmp("user.bar", name) == 0);
	free(name);
	value = sqsh_xattr_iterator_value_dup(xattr_iter);
	assert(value != NULL);
	assert(strcmp(expected_value, value) == 0);
	free(value);
	rv = sqsh_xattr_iterator_next(xattr_iter);
	assert(rv == 0);
	rv = sqsh_xattr_iterator_free(xattr_iter);
	assert(rv == 0);
	rv = sqsh_inode_free(entry_inode);
	assert(rv == 0);

	rv = sqsh_directory_iterator_free(dir_iter);
	assert(rv == 0);

	rv = sqsh__inode_cleanup(&inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
fuzz_crash_1(void) {
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

	struct SqshInode *inode;
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "");
	assert(rv < 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv == 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
fuzz_crash_2(void) {
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

	struct SqshInode *inode = NULL;
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "");
	assert(rv < 0);

	inode = sqsh_tree_walker_inode_load(&walker, &rv);
	assert(inode != NULL);
	assert(rv < 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
fuzz_crash_3(void) {
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

	struct SqshInode *inode = NULL;
	struct SqshArchive sqsh = {0};
	struct SqshTreeWalker walker = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);

	rv = sqsh__tree_walker_init(&walker, &sqsh);
	assert(rv == 0);

	rv = sqsh_tree_walker_resolve(&walker, "");
	assert(rv < 0);

	rv = sqsh_inode_free(inode);
	assert(rv == 0);

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

static void
fuzz_crash_4(void) {
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

	struct SqshIdTable *id_table = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);
	rv = sqsh_archive_id_table(&sqsh, &id_table);
	assert(rv == -SQSH_ERROR_SIZE_MISSMATCH);
	sqsh__archive_cleanup(&sqsh);
}

static void
fuzz_crash_5(void) {
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

	struct SqshIdTable *id_table = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);
	rv = sqsh_archive_id_table(&sqsh, &id_table);
	assert(rv == -SQSH_ERROR_SIZE_MISSMATCH);
	sqsh__archive_cleanup(&sqsh);
}

static void
fuzz_crash_6(void) {
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

	struct SqshIdTable *id_table = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = {
			.source_mapper = sqsh_mapper_impl_static,
			.source_size = sizeof(input),
	};
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);
	rv = sqsh_archive_id_table(&sqsh, &id_table);
	assert(rv == -SQSH_ERROR_SIZE_MISSMATCH);
	sqsh__archive_cleanup(&sqsh);
}

static void
fuzz_crash_7(void) {
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

	struct SqshIdTable *id_table = NULL;
	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = DEFAULT_CONFIG(sizeof(input));
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv == 0);
	rv = sqsh_archive_id_table(&sqsh, &id_table);
	assert(rv == -SQSH_ERROR_SIZE_MISSMATCH);
	sqsh__archive_cleanup(&sqsh);
}

static void
fuzz_crash_8(void) {
	int rv;
	static const uint8_t input[] = {
			0xa, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,
			0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0x0, 0xe4, 0x0,
	};

	struct SqshArchive sqsh = {0};
	const struct SqshConfig config = DEFAULT_CONFIG(sizeof(input));
	rv = sqsh__archive_init(&sqsh, input, &config);
	assert(rv != 0);
	sqsh__archive_cleanup(&sqsh);
}

static void
free_null_crash_1(void) {
	int rv;
	rv = sqsh_archive_free(NULL);
	assert(rv == 0);
	rv = sqsh_file_reader_free(NULL);
	assert(rv == 0);
	rv = sqsh_file_iterator_free(NULL);
	assert(rv == 0);
	rv = sqsh__directory_index_iterator_free(NULL);
	assert(rv == 0);
	sqsh_directory_iterator_free(NULL);
	assert(rv == 0);
	sqsh_xattr_iterator_free(NULL);
	assert(rv == 0);
}

struct Walker {
	struct SqshArchive *sqsh;
	uint64_t inode_number;
};

static void *
multithreaded_walker(void *arg) {
	int rv;
	struct Walker *walker = arg;
	struct Walker my_walker = {
			.sqsh = walker->sqsh,
	};

	struct SqshInode *inode =
			sqsh_inode_new(walker->sqsh, walker->inode_number, &rv);

	if (sqsh_inode_type(inode) == SQSH_INODE_TYPE_DIRECTORY) {
		struct SqshDirectoryIterator *iter =
				sqsh_directory_iterator_new(inode, &rv);
		while (sqsh_directory_iterator_next(iter) > 0) {
			multithreaded_walker(&my_walker);
		}
		sqsh_directory_iterator_free(iter);
	} else {
		struct SqshFileReader *reader = sqsh_file_reader_new(inode, &rv);
		size_t size = sqsh_inode_file_size(inode);
		rv = sqsh_file_reader_advance(reader, 0, size);
		assert(rv == 0);
		sqsh_file_reader_free(reader);
	}

	sqsh_inode_free(inode);

	return 0;
}
static void
multithreaded(void) {
	int rv;
	pthread_t threads[16] = {0};
	struct SqshArchive sqsh = {0};

	const struct SqshConfig config = DEFAULT_CONFIG(test_squashfs_image_len);
	rv = sqsh__archive_init(&sqsh, (char *)test_squashfs_image, &config);
	assert(rv == 0);

	const struct SqshSuperblock *superblock = sqsh_archive_superblock(&sqsh);
	struct Walker walker = {
			.sqsh = &sqsh,
			.inode_number = sqsh_superblock_inode_root_ref(superblock),
	};
	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_create(&threads[i], NULL, multithreaded_walker, &walker);
		assert(rv == 0);
	}

	for (unsigned long i = 0; i < LENGTH(threads); i++) {
		rv = pthread_join(threads[i], NULL);
		assert(rv == 0);
	}

	rv = sqsh__archive_cleanup(&sqsh);
	assert(rv == 0);
}

DEFINE
TEST(sqsh_empty);
TEST(sqsh_ls);
TEST(tree_walker);
TEST(sqsh_get_nonexistant);
TEST(sqsh_cat_fragment);
TEST(sqsh_cat_datablock_and_fragment);
TEST(sqsh_cat_size_overflow);
TEST(sqsh_test_uid_and_gid);
TEST(sqsh_test_extended_dir);
TEST(sqsh_test_xattr);
TEST_OFF(fuzz_crash_1); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_2); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_3); // Fails since the library sets up tables
TEST_OFF(fuzz_crash_4);
TEST_OFF(fuzz_crash_5);
TEST_OFF(fuzz_crash_6);
TEST_OFF(fuzz_crash_7);
TEST(fuzz_crash_8);
TEST(free_null_crash_1);
TEST(multithreaded);
DEFINE_END
