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

#include <sqsh_tree_private.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_directory_private.h>
#include <sqsh_error.h>
#include <sqsh_file_private.h>

#include <stdlib.h>
#include <string.h>

#define SQSH_DEFAULT_MAX_SYMLINKS_FOLLOWED 100

SQSH_NO_UNUSED static int path_resolve(
		struct SqshPathResolver *walker, const char *path, size_t path_len,
		int recursion);

SQSH_NO_UNUSED static int
update_inode_from_iterator(struct SqshPathResolver *walker) {
	// TODO: this should be done from sqsh__file_init.
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	uint32_t inode_number = sqsh_directory_iterator_inode(iterator);
	uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);

	walker->current_inode_ref = inode_ref;
	return sqsh_inode_map_set2(walker->inode_map, inode_number, inode_ref);
}

SQSH_NO_UNUSED static int
enter_directory(struct SqshPathResolver *walker, uint64_t inode_ref) {
	int rv = 0;
	struct SqshFile *cwd = &walker->cwd;
	struct SqshDirectoryIterator *iterator = &walker->iterator;

	rv = sqsh__file_cleanup(cwd);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_cleanup(iterator);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_init(cwd, walker->archive, inode_ref);
	if (rv < 0) {
		goto out;
	}

	if (sqsh_file_type(cwd) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
		goto out;
	}

	walker->begin_iterator = true;
	rv = sqsh__directory_iterator_init(iterator, cwd);
	if (rv < 0) {
		goto out;
	}

	const uint32_t inode_number = sqsh_file_inode(cwd);
	walker->current_inode_ref = inode_ref;
	rv = sqsh_inode_map_set2(walker->inode_map, inode_number, inode_ref);

out:
	return rv;
}

int
sqsh__path_resolver_init(
		struct SqshPathResolver *walker, struct SqshArchive *archive) {
	int rv = 0;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const struct SqshConfig *config = sqsh_archive_config(archive);

	walker->max_symlink_depth = SQSH_CONFIG_DEFAULT(
			config->max_symlink_depth, SQSH_DEFAULT_MAX_SYMLINKS_FOLLOWED);
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

struct SqshPathResolver *
sqsh_path_resolver_new(struct SqshArchive *archive, int *err) {
	SQSH_NEW_IMPL(sqsh__path_resolver_init, struct SqshPathResolver, archive);
}

int
sqsh_path_resolver_up(struct SqshPathResolver *walker) {
	int rv = 0;
	const struct SqshFile *cwd = &walker->cwd;
	/* We do not use the parent inode to check if it is the root node.
	 * According to the documentationen it *should* be zero. That's vague.
	 */

	if (sqsh_file_inode_ref(cwd) == walker->root_inode_ref) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_UP;
	}
	const uint32_t parent_inode = sqsh_file_directory_parent_inode(cwd);
	if (parent_inode <= 0) {
		rv = -SQSH_ERROR_CORRUPTED_INODE;
		goto out;
	}

	const uint64_t parent_inode_ref =
			sqsh_inode_map_get2(walker->inode_map, parent_inode, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = enter_directory(walker, parent_inode_ref);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

bool
sqsh_path_resolver_next(struct SqshPathResolver *walker, int *err) {
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	int rv = 0;
	walker->begin_iterator = false;

	bool has_next = sqsh_directory_iterator_next(iterator, &rv);
	if (rv < 0) {
		goto out;
	}
	if (has_next != false) {
		rv = update_inode_from_iterator(walker);
		if (rv < 0) {
			goto out;
		}
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	if (rv < 0) {
		return false;
	} else {
		return has_next;
	}
}

enum SqshFileType
sqsh_path_resolver_type(const struct SqshPathResolver *walker) {
	if (walker->begin_iterator == true) {
		return sqsh_file_type(&walker->cwd);
	} else {
		return sqsh_directory_iterator_file_type(&walker->iterator);
	}
}

const char *
sqsh_path_resolver_name(const struct SqshPathResolver *walker) {
	if (walker->begin_iterator == true) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name(&walker->iterator);
	}
}

uint16_t
sqsh_path_resolver_name_size(const struct SqshPathResolver *walker) {
	if (walker->begin_iterator == true) {
		return 0;
	} else {
		return sqsh_directory_iterator_name_size(&walker->iterator);
	}
}

char *
sqsh_path_resolver_name_dup(const struct SqshPathResolver *walker) {
	if (walker->begin_iterator == true) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name_dup(&walker->iterator);
	}
}

int
sqsh_path_resolver_revert(struct SqshPathResolver *walker) {
	struct SqshFile *inode = &walker->cwd;
	struct SqshDirectoryIterator *iterator = &walker->iterator;

	sqsh__directory_iterator_cleanup(iterator);
	walker->begin_iterator = true;
	return sqsh__directory_iterator_init(iterator, inode);
}

int
sqsh_path_resolver_lookup(
		struct SqshPathResolver *walker, const char *name,
		const size_t name_size) {
	int rv = 0;
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	rv = sqsh_path_resolver_revert(walker);
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
sqsh_path_resolver_down(struct SqshPathResolver *walker) {
	if (walker->begin_iterator == true) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_DOWN;
	}
	const uint64_t child_inode_ref =
			sqsh_directory_iterator_inode_ref(&walker->iterator);

	return enter_directory(walker, child_inode_ref);
}

int
sqsh_path_resolver_to_root(struct SqshPathResolver *walker) {
	return enter_directory(walker, walker->root_inode_ref);
}

struct SqshFile *
sqsh_path_resolver_open_file(const struct SqshPathResolver *walker, int *err) {
	return sqsh_open_by_ref(walker->archive, walker->current_inode_ref, err);
}

static size_t
path_segment_len(const char *path, size_t path_len) {
	sqsh_index_t len = 0;
	for (; len < path_len && path[len] != '/'; len++) {
	}
	return len;
}

static const char *
path_next_segment(const char *path, size_t path_len) {
	sqsh_index_t current_segment_len = path_segment_len(path, path_len);
	if (current_segment_len == path_len) {
		return NULL;
	} else {
		return &path[current_segment_len] + 1;
	}
}

static int
path_resolver_follow_symlink(struct SqshPathResolver *walker, int recursion) {
	struct SqshFile inode = {0};
	int rv = 0;
	// if symlinks_followed is smaller than zero, the caller intends to disable
	// symlink following
	if (recursion < 0) {
		return 0;
	}
	if (recursion == 0) {
		return -SQSH_ERROR_TOO_MANY_SYMLINKS_FOLLOWED;
	}

	rv = sqsh__file_init(&inode, walker->archive, walker->current_inode_ref);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_path_resolver_revert(walker);
	if (rv < 0) {
		goto out;
	}

	const char *symlink = sqsh_file_symlink(&inode);
	size_t symlink_len = sqsh_file_symlink_size(&inode);
	rv = path_resolve(walker, symlink, symlink_len, recursion - 1);

out:
	sqsh__file_cleanup(&inode);
	return rv;
}

static int
path_resolve(
		struct SqshPathResolver *walker, const char *path, size_t path_len,
		int recursion) {
	int rv = 0;
	const char *segment = path, *next_segment;
	size_t remaining_path_len = path_len;
	bool is_dir = true;
	if (segment[0] == '/') {
		rv = sqsh_path_resolver_to_root(walker);
		if (rv < 0) {
			goto out;
		}
	}

	do {
		const size_t segment_len =
				path_segment_len(segment, remaining_path_len);
		next_segment = path_next_segment(segment, remaining_path_len);
		remaining_path_len -= segment_len + 1;
		if (strncmp(".", segment, segment_len) == 0 || segment_len == 0) {
			is_dir = true;
			continue;
		} else if (strncmp("..", segment, segment_len) == 0) {
			is_dir = true;
			rv = sqsh_path_resolver_up(walker);
		} else {
			rv = sqsh_path_resolver_lookup(walker, segment, segment_len);
			if (rv < 0) {
				goto out;
			}
			switch (sqsh_path_resolver_type(walker)) {
			case SQSH_FILE_TYPE_SYMLINK:
				rv = path_resolver_follow_symlink(walker, recursion);
				if (rv < 0) {
					goto out;
				}
				is_dir = sqsh_path_resolver_type(walker) ==
						SQSH_FILE_TYPE_DIRECTORY;
				break;
			case SQSH_FILE_TYPE_DIRECTORY:
				is_dir = true;
				rv = sqsh_path_resolver_down(walker);
				break;
			default:
				is_dir = false;
				break;
			}
		}
		if (rv < 0) {
			goto out;
		}
	} while ((segment = next_segment) && is_dir);
	if (segment != NULL && is_dir == false) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
	}

out:
	return rv;
}

int
sqsh_path_resolver_resolve(
		struct SqshPathResolver *walker, const char *path, bool follow_links) {
	int recursion = follow_links ? (int)walker->max_symlink_depth : -1;
	return path_resolve(walker, path, strlen(path), recursion);
}

int
sqsh__path_resolver_cleanup(struct SqshPathResolver *walker) {
	sqsh__file_cleanup(&walker->cwd);
	sqsh__directory_iterator_cleanup(&walker->iterator);
	return 0;
}

int
sqsh_path_resolver_free(struct SqshPathResolver *walker) {
	SQSH_FREE_IMPL(sqsh__path_resolver_cleanup, walker);
}
