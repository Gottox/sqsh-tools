/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         walker.c
 */

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_directory_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode_private.h"
#include "../../include/sqsh_tree_private.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int
load_inode(struct SqshTreeWalker *walker, uint64_t inode_ref) {
	int rv = 0;
	struct SqshInode *inode = &walker->inode;
	struct SqshDirectoryIterator *iterator = &walker->iterator;

	rv = sqsh__inode_cleanup(inode);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_cleanup(iterator);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__inode_init(inode, walker->archive, inode_ref);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_init(iterator, inode);
	if (rv < 0) {
		goto out;
	}

	const uint64_t inode_number = sqsh_inode_number(inode);
	walker->inode_ref_table[inode_number - 1] = inode_ref;

out:
	return rv;
}

int
sqsh__tree_walker_init(
		struct SqshTreeWalker *walker, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint64_t inode_count = sqsh_superblock_inode_count(superblock);

	walker->archive = archive;
	walker->root_inode_ref = sqsh_superblock_inode_root_ref(superblock);
	walker->inode_ref_table = calloc(inode_count, sizeof(uint64_t));
	if (walker->inode_ref_table == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = load_inode(walker, walker->root_inode_ref);

out:
	return rv;
}

struct SqshTreeWalker *
sqsh_tree_walker_new(struct SqshArchive *archive, int *err) {
	struct SqshTreeWalker *walker = calloc(1, sizeof(struct SqshTreeWalker));
	if (walker == NULL) {
		return NULL;
	}
	*err = sqsh__tree_walker_init(walker, archive);
	if (*err < 0) {
		free(walker);
		return NULL;
	}
	return walker;
}

int
sqsh_tree_walker_up(struct SqshTreeWalker *walker) {
	int rv = 0;
	const struct SqshInode *inode = &walker->inode;
	// We do not use the parent inode to check if it is the root node.
	// According to the documentationen it *should* be zero. That's
	// vague.

	if (sqsh_inode_ref(inode) == walker->root_inode_ref) {
		return -SQSH_ERROR_TODO;
	}
	const uint64_t parent_inode = sqsh_inode_directory_parent_inode(inode);
	if (parent_inode <= 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	const uint64_t parent_inode_ref = walker->inode_ref_table[parent_inode - 1];
	if (parent_inode_ref == 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}

	rv = load_inode(walker, parent_inode_ref);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh_tree_walker_next(struct SqshTreeWalker *walker) {
	return sqsh_directory_iterator_next(&walker->iterator);
}

const char *
sqsh_tree_walker_name(const struct SqshTreeWalker *walker) {
	return sqsh_directory_iterator_name(&walker->iterator);
}

uint16_t
sqsh_tree_walker_name_size(const struct SqshTreeWalker *walker) {
	return sqsh_directory_iterator_name_size(&walker->iterator);
}

char *
sqsh_tree_walker_name_dup(const struct SqshTreeWalker *walker) {
	return sqsh_directory_iterator_name_dup(&walker->iterator);
}

int
sqsh_tree_walker_lookup(
		struct SqshTreeWalker *walker, const char *name,
		const size_t name_size) {
	return sqsh_directory_iterator_lookup(&walker->iterator, name, name_size);
}

int
sqsh_tree_walker_down(struct SqshTreeWalker *walker) {
	const uint64_t child_inode_ref =
			sqsh_directory_iterator_inode_ref(&walker->iterator);

	return load_inode(walker, child_inode_ref);
}

int
sqsh_tree_walker_to_root(struct SqshTreeWalker *walker) {
	return load_inode(walker, walker->root_inode_ref);
}

struct SqshInode *
sqsh_tree_walker_inode_load(struct SqshTreeWalker *walker, int *err) {
	return sqsh_directory_iterator_inode_load(&walker->iterator, err);
}

static const char *
path_next_segment(const char *segment) {
	segment = strchr(segment, '/');
	if (segment == NULL) {
		return NULL;
	} else {
		return segment + 1;
	}
}

size_t
path_segment_len(const char *segment) {
	const char *next_segment = path_next_segment(segment);
	if (next_segment == NULL) {
		return strlen(segment);
	} else {
		return next_segment - segment - 1;
	}
}

int
sqsh_tree_walker_resolve(struct SqshTreeWalker *walker, const char *path) {
	int rv = 0;
	const char *segment = path;
	if (segment[0] == '/') {
		rv = sqsh_tree_walker_to_root(walker);
		if (rv < 0) {
			goto out;
		}
	}

	do {
		const size_t segment_len = path_segment_len(segment);
		if (strncmp(".", segment, segment_len) == 0 || segment_len == 0) {
			continue;
		} else if (strncmp("..", segment, segment_len) == 0) {
			rv = sqsh_tree_walker_up(walker);
		} else {
			rv = sqsh_tree_walker_lookup(walker, segment, segment_len);
			if (rv < 0) {
				goto out;
			}
			if (path_next_segment(segment) != NULL) {
				rv = sqsh_tree_walker_down(walker);
			}
		}
		if (rv < 0) {
			goto out;
		}
	} while ((segment = path_next_segment(segment)));

out:
	return rv;
}

int
sqsh__tree_walker_cleanup(struct SqshTreeWalker *walker) {
	sqsh__inode_cleanup(&walker->inode);
	sqsh__directory_iterator_cleanup(&walker->iterator);
	free(walker->inode_ref_table);
	return 0;
}

int
sqsh_tree_walker_free(struct SqshTreeWalker *walker) {
	if (walker == NULL) {
		return 0;
	}
	int rv = sqsh__tree_walker_cleanup(walker);
	free(walker);
	return rv;
}
