/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include <sqsh.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_CHUNK_SIZE 4096

static int
read_file(struct SqshTreeTraversal *traversal) {
	struct SqshFile *inode = NULL;
	struct SqshFileReader *file = NULL;
	struct SqshXattrIterator *xattr_iter = NULL;
	int rv;

	inode = sqsh_tree_traversal_open_file(traversal, &rv);
	if (rv < 0) {
		goto out;
	}

	file = sqsh_file_reader_new(inode, &rv);
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
		rv = sqsh_file_reader_advance(file, advance, chunk_size);
		if (rv < 0) {
			goto out;
		}
	}

	if (rv < 0) {
		goto out;
	}

	xattr_iter = sqsh_xattr_iterator_new(inode, &rv);
	if (rv < 0) {
		goto out;
	}
	while (sqsh_xattr_iterator_next(xattr_iter, &rv)) {
		char *fullname = sqsh_xattr_iterator_fullname_dup(xattr_iter);
		free(fullname);
		sqsh_xattr_iterator_value(xattr_iter);
	}

out:
	sqsh_xattr_iterator_free(xattr_iter);
	sqsh_file_reader_free(file);
	sqsh_close(inode);
	return 0;
}

int
LLVMFuzzerTestOneInput(char *data, size_t size) {
	int rv = 0;
	struct SqshArchive *archive = NULL;
	struct SqshFile *inode = NULL;
	struct SqshTreeTraversal *traversal = NULL;
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

	traversal = sqsh_tree_traversal_new(inode, &rv);
	if (traversal == NULL) {
		goto out;
	}
	while (sqsh_tree_traversal_next(traversal, &rv)) {
		if (read_file(traversal) < 0)
			break;
	}

out:

	sqsh_tree_traversal_free(traversal);
	sqsh_close(inode);
	sqsh_archive_close(archive);

	(void)rv;
	return 0; // Non-zero return values are reserved for future use.
}
