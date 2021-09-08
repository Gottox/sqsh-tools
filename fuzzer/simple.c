/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/squash.h"
#include <stdint.h>

int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	struct Squash squash = {0};
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	squash_init(&squash, (uint8_t *)data, size, SQUASH_DTOR_NONE);

	squash_inode_load_ref(&inode, &squash,
			squash_superblock_root_inode_ref(squash.superblock));

	squash_directory_init(&dir, &squash, &inode);
	squash_directory_iterator_init(&iter, &dir);
	while (squash_directory_iterator_next(&iter)) {
	}
	squash_directory_iterator_clean(&iter);

	return 0; // Non-zero return values are reserved for future use.
}
