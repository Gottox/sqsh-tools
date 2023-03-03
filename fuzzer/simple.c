/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_context.h"
#include "../include/sqsh_directory_private.h"
#include "../include/sqsh_file_private.h"
#include "../include/sqsh_inode_private.h"
#include <stdint.h>

static int
read_file(struct SqshInodeContext *inode) {
	struct SqshFileContext file = {0};
	int rv;
	rv = sqsh__file_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_file_read(&file, sqsh_inode_file_size(inode));
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__file_cleanup(&file);
	// noop
	return 0;
}
int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct SqshArchive sqsh = {0};
	struct SqshInodeContext inode = {0};
	struct SqshDirectoryIterator iter = {0};
	const struct SqshSuperblockContext *superblock;
	rv = sqsh__archive_init(
			&sqsh, (uint8_t *)data,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = size,
			});
	if (rv < 0) {
		goto out;
	}

	superblock = sqsh_archive_superblock(&sqsh);
	uint64_t inode_ref = sqsh_superblock_inode_root_ref(superblock);
	rv = sqsh__inode_init(&inode, &sqsh, inode_ref);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_init(&iter, &inode);
	if (rv < 0) {
		goto out;
	}
	while (sqsh_directory_iterator_next(&iter)) {
		if (read_file(&inode) < 0)
			break;
	}

out:

	sqsh__directory_iterator_cleanup(&iter);
	sqsh__inode_cleanup(&inode);
	sqsh__archive_cleanup(&sqsh);

	return rv; // Non-zero return values are reserved for future use.
}
