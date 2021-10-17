/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../src/context/directory_context.h"
#include "../src/context/file_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/superblock_context.h"
#include "../src/squash.h"
#include <stdint.h>

static int
read_file(struct SquashInodeContext *inode) {
	struct SquashFileContext file = {0};
	int rv;
	rv = squash_file_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	rv = squash_file_read(&file, squash_inode_file_size(inode));
	if (rv < 0) {
		goto out;
	}

out:
	squash_file_cleanup(&file);
	// noop
	return 0;
}
int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct Squash squash = {0};
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	rv = squash_init(&squash, (uint8_t *)data, size, SQUASH_DTOR_NONE);
	if (rv < 0) {
		goto out;
	}

	rv = squash_inode_load(
			&inode, &squash.superblock,
			squash_superblock_inode_root_ref(&squash.superblock));
	if (rv < 0) {
		goto out;
	}

	rv = squash_directory_init(&dir, &squash.superblock, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		goto out;
	}
	while (squash_directory_iterator_next(&iter)) {
		if (read_file(&inode) < 0)
			break;
	}

out:

	squash_directory_iterator_cleanup(&iter);
	squash_directory_cleanup(&dir);
	squash_inode_cleanup(&inode);
	squash_inode_cleanup(&inode);

	return rv; // Non-zero return values are reserved for future use.
}
