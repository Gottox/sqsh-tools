/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/data/superblock.h"
#include "../src/squash.h"
#include <stdint.h>

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

	rv = squash_inode_load_ref(&inode, squash.superblock,
			squash_data_superblock_root_inode_ref(squash.superblock));
	if (rv < 0) {
		goto out;
	}

	rv = squash_directory_init(&dir, squash.superblock, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		goto out;
	}
	while (squash_directory_iterator_next(&iter)) {
		// noop
	}

out:

	squash_directory_iterator_clean(&iter);
	squash_directory_cleanup(&dir);
	squash_inode_cleanup(&inode);

	return rv; // Non-zero return values are reserved for future use.
}
