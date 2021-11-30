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
#include "../src/hsqs.h"
#include <stdint.h>

static int
read_file(struct HsqsInodeContext *inode) {
	struct HsqsFileContext file = {0};
	int rv;
	rv = hsqs_file_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_file_read(&file, hsqs_inode_file_size(inode));
	if (rv < 0) {
		goto out;
	}

out:
	hsqs_file_cleanup(&file);
	// noop
	return 0;
}
int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct Hsqs hsqs = {0};
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryContext dir = {0};
	struct HsqsDirectoryIterator iter = {0};
	rv = hsqs_init(&hsqs, (uint8_t *)data, size);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_inode_load(
			&inode, &hsqs.superblock,
			hsqs_superblock_inode_root_ref(&hsqs.superblock));
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_directory_init(&dir, &hsqs.superblock, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		goto out;
	}
	while (hsqs_directory_iterator_next(&iter)) {
		if (read_file(&inode) < 0)
			break;
	}

out:

	hsqs_directory_iterator_cleanup(&iter);
	hsqs_directory_cleanup(&dir);
	hsqs_inode_cleanup(&inode);
	hsqs_cleanup(&hsqs);

	return rv; // Non-zero return values are reserved for future use.
}
