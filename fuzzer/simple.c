/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../src/context/content_context.h"
#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/superblock_context.h"
#include "../src/sqsh.h"
#include <stdint.h>

static int
read_file(struct SqshInodeContext *inode) {
	struct SqshFileContext file = {0};
	int rv;
	rv = sqsh_content_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_content_read(&file, sqsh_inode_file_size(inode));
	if (rv < 0) {
		goto out;
	}

out:
	sqsh_content_cleanup(&file);
	// noop
	return 0;
}
int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct Sqsh sqsh = {0};
	struct SqshInodeContext inode = {0};
	struct SqshDirectoryContext dir = {0};
	struct SqshDirectoryIterator iter = {0};
	rv = sqsh_init(&sqsh, (uint8_t *)data, size);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_inode_load(
			&inode, &sqsh.superblock,
			sqsh_superblock_inode_root_ref(&sqsh.superblock));
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_directory_init(&dir, &sqsh.superblock, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		goto out;
	}
	while (sqsh_directory_iterator_next(&iter)) {
		if (read_file(&inode) < 0)
			break;
	}

out:

	sqsh_directory_iterator_cleanup(&iter);
	sqsh_directory_cleanup(&dir);
	sqsh_inode_cleanup(&inode);
	sqsh_cleanup(&sqsh);

	return rv; // Non-zero return values are reserved for future use.
}
