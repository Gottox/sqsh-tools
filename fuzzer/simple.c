/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include <sqsh_context.h>
#include <sqsh_iterator.h>
#include <sqsh_private.h>

static int
read_file(struct SqshInodeContext *inode) {
	struct SqshFileContext file = {0};
	int rv;
	rv = sqsh_file_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_file_read(&file, sqsh_inode_file_size(inode));
	if (rv < 0) {
		goto out;
	}

out:
	sqsh_file_cleanup(&file);
	// noop
	return 0;
}
int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct Sqsh sqsh = {0};
	struct SqshInodeContext inode = {0};
	struct SqshDirectoryIterator iter = {0};
	rv = sqsh__init(
			&sqsh, (uint8_t *)data,
			&(struct SqshConfig){
					.source_type = SQSH_SOURCE_TYPE_MEMORY,
					.source_size = size,
			});
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_inode_init_root(&inode, &sqsh);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_directory_iterator_init(&iter, &inode);
	if (rv < 0) {
		goto out;
	}
	while (sqsh_directory_iterator_next(&iter)) {
		if (read_file(&inode) < 0)
			break;
	}

out:

	sqsh_directory_iterator_cleanup(&iter);
	sqsh_inode_cleanup(&inode);
	sqsh__cleanup(&sqsh);

	return rv; // Non-zero return values are reserved for future use.
}
