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

#include "../../include/sqsh_tree_private.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_directory_private.h"
#include "../../include/sqsh_error.h"
#include "../../include/sqsh_inode_private.h"
#include "../utils/utils.h"

#include <stdlib.h>
#include <string.h>

SQSH_NO_UNUSED int
update_inode_from_iterator(struct SqshTreeWalker *walker) {
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	uint64_t inode_number = sqsh_directory_iterator_inode_number(iterator);
	uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);

	walker->current_inode_ref = inode_ref;
	return sqsh_inode_map_set(walker->inode_map, inode_number, inode_ref);
}

static int
enter_directory(struct SqshTreeWalker *walker, uint64_t inode_ref) {
	int rv = 0;
	struct SqshInode *inode = &walker->directory;
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

	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_DIRECTORY) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
		goto out;
	}

	walker->begin_iterator = true;
	rv = sqsh__directory_iterator_init(iterator, inode);
	if (rv < 0) {
		goto out;
	}

	const uint64_t inode_number = sqsh_inode_number(inode);
	walker->current_inode_ref = inode_ref;
	rv = sqsh_inode_map_set(walker->inode_map, inode_number, inode_ref);

out:
	return rv;
}

int
sqsh__tree_walker_init(
		struct SqshTreeWalker *walker, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

	walker->archive = archive;
	walker->root_inode_ref = sqsh_superblock_inode_root_ref(superblock);
	walker->begin_iterator = true;
	rv = sqsh_archive_inode_map(archive, &walker->inode_map);
	if (rv < 0) {
		goto out;
	}
	rv = enter_directory(walker, walker->root_inode_ref);

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
	const struct SqshInode *inode = &walker->directory;
	/* We do not use the parent inode to check if it is the root node.
	 * According to the documentationen it *should* be zero. That's vague.
	 */

	if (sqsh_inode_ref(inode) == walker->root_inode_ref) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_UP;
	}
	const uint64_t parent_inode = sqsh_inode_directory_parent_inode(inode);
	if (parent_inode <= 0) {
		rv = -SQSH_ERROR_CORRUPTED_INODE;
		goto out;
	}
	const uint64_t parent_inode_ref =
			sqsh_inode_map_get(walker->inode_map, parent_inode);
	if (parent_inode_ref == 0) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	rv = enter_directory(walker, parent_inode_ref);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh_tree_walker_next(struct SqshTreeWalker *walker) {
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	int rv = sqsh_directory_iterator_next(iterator);
	walker->begin_iterator = false;
	if (rv < 0) {
		return rv;
	}

	return update_inode_from_iterator(walker);
}

enum SqshInodeType
sqsh_tree_walker_type(const struct SqshTreeWalker *walker) {
	if (walker->begin_iterator == true) {
		return sqsh_inode_type(&walker->directory);
	} else {
		return sqsh_directory_iterator_inode_type(&walker->iterator);
	}
}

const char *
sqsh_tree_walker_name(const struct SqshTreeWalker *walker) {
	if (walker->begin_iterator == true) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name(&walker->iterator);
	}
}

uint16_t
sqsh_tree_walker_name_size(const struct SqshTreeWalker *walker) {
	if (walker->begin_iterator == true) {
		return 0;
	} else {
		return sqsh_directory_iterator_name_size(&walker->iterator);
	}
}

char *
sqsh_tree_walker_name_dup(const struct SqshTreeWalker *walker) {
	if (walker->begin_iterator == true) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name_dup(&walker->iterator);
	}
}

int
sqsh_tree_walker_revert(struct SqshTreeWalker *walker) {
	struct SqshInode *inode = &walker->directory;
	struct SqshDirectoryIterator *iterator = &walker->iterator;

	sqsh__directory_iterator_cleanup(iterator);
	walker->begin_iterator = true;
	return sqsh__directory_iterator_init(iterator, inode);
}

int
sqsh_tree_walker_lookup(
		struct SqshTreeWalker *walker, const char *name,
		const size_t name_size) {
	int rv = 0;
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	rv = sqsh_tree_walker_revert(walker);
	if (rv < 0) {
		return rv;
	}
	rv = sqsh_directory_iterator_lookup(iterator, name, name_size);
	if (rv < 0) {
		return rv;
	}
	walker->begin_iterator = false;

	return update_inode_from_iterator(walker);
}

int
sqsh_tree_walker_down(struct SqshTreeWalker *walker) {
	if (walker->begin_iterator == true) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_DOWN;
	}
	const uint64_t child_inode_ref =
			sqsh_directory_iterator_inode_ref(&walker->iterator);

	return enter_directory(walker, child_inode_ref);
}

int
sqsh_tree_walker_to_root(struct SqshTreeWalker *walker) {
	return enter_directory(walker, walker->root_inode_ref);
}

struct SqshInode *
sqsh_tree_walker_inode_load(const struct SqshTreeWalker *walker, int *err) {
	return sqsh_inode_new(walker->archive, walker->current_inode_ref, err);
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

static int
tree_walker_resolve(struct SqshTreeWalker *walker, const char *path) {
	int rv = 0;
	const char *segment = path, *next_segment;
	if (segment[0] == '/') {
		rv = sqsh_tree_walker_to_root(walker);
		if (rv < 0) {
			goto out;
		}
	}

	do {
		const size_t segment_len = path_segment_len(segment);
		next_segment = path_next_segment(segment);
		if (strncmp(".", segment, segment_len) == 0 || segment_len == 0) {
			continue;
		} else if (strncmp("..", segment, segment_len) == 0) {
			rv = sqsh_tree_walker_up(walker);
		} else {
			rv = sqsh_tree_walker_lookup(walker, segment, segment_len);
			if (rv < 0) {
				goto out;
			}
			if (sqsh_tree_walker_type(walker) == SQSH_INODE_TYPE_DIRECTORY) {
				rv = sqsh_tree_walker_down(walker);
			}
		}
		if (rv < 0) {
			goto out;
		}
	} while ((segment = next_segment));

out:
	return rv;
}

int
sqsh_tree_walker_resolve(
		struct SqshTreeWalker *walker, const char *path, bool follow_links) {
	int rv = 0;

	char *current_path_buf = NULL;
	const char *current_path = path;
	for (int i = 0; i < 100; i++) {
		rv = tree_walker_resolve(walker, current_path);
		if (rv < 0) {
			goto out;
		}
		if (!follow_links) {
			break;
		}
		if (sqsh_tree_walker_type(walker) != SQSH_INODE_TYPE_SYMLINK) {
			break;
		}
		struct SqshInode inode = {0};
		rv = sqsh__inode_init(
				&inode, walker->archive, walker->current_inode_ref);
		if (rv < 0) {
			goto out;
		}
		free(current_path_buf);
		current_path = current_path_buf = sqsh_inode_symlink_dup(&inode);
		sqsh__inode_cleanup(&inode);
	}
out:
	free(current_path_buf);
	return rv;
}

int
sqsh__tree_walker_cleanup(struct SqshTreeWalker *walker) {
	sqsh__inode_cleanup(&walker->directory);
	sqsh__directory_iterator_cleanup(&walker->iterator);
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
