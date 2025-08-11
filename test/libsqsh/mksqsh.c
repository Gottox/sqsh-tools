/******************************************************************************
 *                                                                            *
 * Copyright (c) 2024, Enno Boland <g@s01.de>                                 *
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
 * @file         mksqsh.c
 */

#define _GNU_SOURCE
#include <mksqsh_archive_private.h>
#include <mksqsh_file.h>
#include <mksqsh_file_private.h>
#include <sqsh.h>
#include <testlib.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
create_and_read(void) {
	static const char content[] = "hello";
	struct MksqshArchive archive = {0};
	int rv = mksqsh_archive_init(&archive);
	ASSERT_EQ(0, rv);

	struct MksqshFile *root = mksqsh_archive_root(&archive);
	ASSERT_NE(root, NULL);
	struct MksqshFile *file =
			mksqsh_file_new(&archive, MKSQSH_FILE_TYPE_REG, &rv);
	ASSERT_NE(file, NULL);
	ASSERT_EQ(0, rv);

	char src_path[] = "/tmp/mksqsh_srcXXXXXX";
	int src_fd = mkstemp(src_path);
	ASSERT(src_fd >= 0);
	ssize_t written = write(src_fd, content, sizeof(content) - 1);
	ASSERT_EQ(sizeof(content) - 1, (size_t)written);
	close(src_fd);
	mksqsh_file_content_from_path(file, src_path, &rv);
	ASSERT_EQ(0, rv);

	rv = mksqsh_file_add(root, "hello.txt", file);
	ASSERT_EQ(0, rv);
	mksqsh_file_release(root);
	mksqsh_file_release(file);

	char path[] = "/tmp/mksqshXXXXXX";
	int fd = mkstemp(path);
	ASSERT(fd >= 0);
	close(fd);

	rv = mksqsh_archive_write(&archive, path);
	ASSERT_EQ(0, rv);
	mksqsh_archive_cleanup(&archive);
	unlink(src_path);

	struct SqshArchive *sqsh = sqsh_archive_open(path, NULL, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(sqsh != NULL);

	uint8_t *sqsh_data = sqsh_easy_file_content(sqsh, "/hello.txt", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(sqsh_data != NULL);
	uint64_t sqsh_size = sqsh_easy_file_size2(sqsh, "/hello.txt", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(sizeof(content) - 1, (size_t)sqsh_size);
	ASSERT_EQ(0, memcmp(sqsh_data, content, sizeof(content) - 1));

	free(sqsh_data);
	sqsh_archive_close(sqsh);
	unlink(path);
}

static void
children_sorted(void) {
	struct MksqshArchive archive = {0};
	int rv = mksqsh_archive_init(&archive);
	ASSERT_EQ(0, rv);

	struct MksqshFile *root = mksqsh_archive_root(&archive);
	ASSERT_NE(root, NULL);

	struct MksqshFile *a = mksqsh_file_new(&archive, MKSQSH_FILE_TYPE_REG, &rv);
	ASSERT_NE(a, NULL);
	ASSERT_EQ(0, rv);

	struct MksqshFile *b = mksqsh_file_new(&archive, MKSQSH_FILE_TYPE_REG, &rv);
	ASSERT_NE(b, NULL);
	ASSERT_EQ(0, rv);

	rv = mksqsh_file_add(root, "b.txt", b);
	ASSERT_EQ(0, rv);
	rv = mksqsh_file_add(root, "a.txt", a);
	ASSERT_EQ(0, rv);

	ASSERT_NE(root->children, NULL);
	ASSERT_EQ(root->children->file, a);
	ASSERT_EQ(root->children->next->file, b);

	mksqsh_file_release(root);
	mksqsh_file_release(a);
	mksqsh_file_release(b);
	mksqsh_archive_cleanup(&archive);
}

static void
duplicate_file(void) {
	struct MksqshArchive archive = {0};
	int rv = mksqsh_archive_init(&archive);
	ASSERT_EQ(0, rv);

	struct MksqshFile *root = mksqsh_archive_root(&archive);
	ASSERT_NE(root, NULL);

	struct MksqshFile *a = mksqsh_file_new(&archive, MKSQSH_FILE_TYPE_REG, &rv);
	ASSERT_NE(a, NULL);
	ASSERT_EQ(0, rv);

	mksqsh_file_content(a, "data", 4);

	rv = mksqsh_file_add(root, "a1.txt", a);
	ASSERT_EQ(0, rv);
	rv = mksqsh_file_add(root, "a2.txt", a);
	ASSERT_EQ(0, rv);

	mksqsh_file_release(root);
	mksqsh_file_release(a);

	char path[] = "/tmp/mksqshXXXXXX";
	int fd = mkstemp(path);
	ASSERT(fd >= 0);
	close(fd);

	rv = mksqsh_archive_write(&archive, path);
	ASSERT_EQ(0, rv);
	mksqsh_archive_cleanup(&archive);

	struct SqshArchive *sqsh = sqsh_archive_open(path, NULL, &rv);
	ASSERT_EQ(0, rv);
	ASSERT_TRUE(sqsh != NULL);

	uint8_t *c1 = sqsh_easy_file_content(sqsh, "/a1.txt", &rv);
	ASSERT_EQ(0, rv);
	uint8_t *c2 = sqsh_easy_file_content(sqsh, "/a2.txt", &rv);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(0, memcmp(c1, c2, 4));

	free(c1);
	free(c2);
	sqsh_archive_close(sqsh);
	unlink(path);
}

DECLARE_TESTS
TEST(create_and_read)
TEST(children_sorted)
TEST(duplicate_file)
END_TESTS
