/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_directory_private.h"
#include "../include/sqsh_file_private.h"
#include "../include/sqsh_inode_private.h"
#include <stdint.h>

static int
read_file(struct SqshDirectoryIterator *iter) {
	struct SqshInode *inode = NULL;
	struct SqshFileReader file = {0};
	int rv;

	inode = sqsh_directory_iterator_inode_load(iter, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_reader_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	size_t file_size = sqsh_inode_file_size(inode);
	rv = sqsh_file_reader_advance(&file, 0, file_size);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh_inode_free(inode);
	sqsh__file_reader_cleanup(&file);
	return 0;
}

int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct SqshArchive *archive = NULL;
	struct SqshInode *inode = NULL;
	struct SqshDirectoryIterator *iter = NULL;
	const struct SqshSuperblock *superblock = NULL;

	archive = sqsh_archive_new(
			(uint8_t *)data,
			&(struct SqshConfig){
					.source_mapper = sqsh_mapper_impl_static,
					.source_size = size,
			},
			&rv);
	if (rv < 0) {
		goto out;
	}

	superblock = sqsh_archive_superblock(archive);
	uint64_t inode_ref = sqsh_superblock_inode_root_ref(superblock);
	inode = sqsh_inode_new(archive, inode_ref, &rv);
	if (rv < 0) {
		goto out;
	}

	iter = sqsh_directory_iterator_new(inode, &rv);
	if (iter == NULL) {
		goto out;
	}
	while ((rv = sqsh_directory_iterator_next(iter)) > 0) {
		if (read_file(iter) < 0)
			break;
	}

out:

	sqsh_directory_iterator_free(iter);
	sqsh_inode_free(inode);
	sqsh_archive_free(archive);

	(void)rv;
	return 0; // Non-zero return values are reserved for future use.
}
