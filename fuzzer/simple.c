/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_directory_private.h"
#include "../include/sqsh_file_private.h"
#include "../include/sqsh_xattr_private.h"
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_CHUNK_SIZE 4096

static int
read_file(struct SqshDirectoryIterator *iter) {
	struct SqshFile *inode = NULL;
	struct SqshFileReader file = {0};
	struct SqshXattrIterator xattr_iter = {0};
	int rv;

	inode = sqsh_directory_iterator_open_file(iter, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_reader_init(&file, inode);
	if (rv < 0) {
		goto out;
	}

	size_t size = sqsh_file_size(inode);
	size_t chunk_size = 0;
	for (; size > 0; size -= chunk_size) {
		const size_t advance = chunk_size;
		chunk_size = DEFAULT_CHUNK_SIZE;
		if (chunk_size > size) {
			chunk_size = size;
		}
		rv = sqsh_file_reader_advance(&file, advance, chunk_size);
		if (rv < 0) {
			goto out;
		}
	}

	if (rv < 0) {
		goto out;
	}

	rv = sqsh__xattr_iterator_init(&xattr_iter, inode);
	if (rv < 0) {
		goto out;
	}
	while (sqsh_xattr_iterator_next(&xattr_iter, &rv)) {
		char *fullname = sqsh_xattr_iterator_fullname_dup(&xattr_iter);
		free(fullname);
		sqsh_xattr_iterator_value(&xattr_iter);
	}

out:
	sqsh__xattr_iterator_cleanup(&xattr_iter);
	sqsh_close(inode);
	sqsh__file_reader_cleanup(&file);
	return 0;
}

int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct SqshArchive *archive = NULL;
	struct SqshFile *inode = NULL;
	struct SqshDirectoryIterator *iter = NULL;
	const struct SqshSuperblock *superblock = NULL;

	archive = sqsh_archive_open(
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
	inode = sqsh_open_by_ref(archive, inode_ref, &rv);
	if (rv < 0) {
		goto out;
	}

	iter = sqsh_directory_iterator_new(inode, &rv);
	if (iter == NULL) {
		goto out;
	}
	while (sqsh_directory_iterator_next(iter, &rv)) {
		if (read_file(iter) < 0)
			break;
	}

out:

	sqsh_directory_iterator_free(iter);
	sqsh_close(inode);
	sqsh_archive_close(archive);

	(void)rv;
	return 0; // Non-zero return values are reserved for future use.
}
