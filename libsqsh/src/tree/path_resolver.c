/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         path_resolver.c
 */

#include <sqsh_tree_private.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_directory_private.h>
#include <sqsh_error.h>
#include <sqsh_file_private.h>

#include <stdlib.h>

#define SQSH_DEFAULT_MAX_SYMLINKS_FOLLOWED 100

static int path_resolve(
		struct SqshPathResolver *resolver, const char *path, size_t path_len,
		bool follow_symlinks);

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

static bool
is_beginning(const struct SqshPathResolver *resolver) {
	return resolver->current_inode_ref == sqsh_file_inode_ref(&resolver->cwd);
}

SQSH_NO_UNUSED static int
update_inode_from_iterator(struct SqshPathResolver *resolver) {
	struct SqshDirectoryIterator *iterator = &resolver->iterator;
	const uint32_t inode_number = sqsh_directory_iterator_inode(iterator);
	const uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);

	if (sqsh_file_inode(&resolver->cwd) == inode_number) {
		return -SQSH_ERROR_CORRUPTED_INODE;
	}
	if (sqsh_file_inode_ref(&resolver->cwd) == inode_ref) {
		return -SQSH_ERROR_CORRUPTED_INODE;
	}

	resolver->current_inode_ref = inode_ref;
	return 0;
}

SQSH_NO_UNUSED static int
update_inode_from_cwd(struct SqshPathResolver *resolver) {
	int rv = 0;
	struct SqshDirectoryIterator *iterator = &resolver->iterator;
	struct SqshFile *cwd = &resolver->cwd;
	const uint64_t inode_ref = sqsh_file_inode_ref(cwd);

	rv = sqsh__directory_iterator_cleanup(iterator);
	if (rv < 0) {
		return rv;
	}

	rv = sqsh__directory_iterator_init(iterator, cwd);
	if (rv < 0) {
		return rv;
	}

	resolver->current_inode_ref = inode_ref;
	return 0;
}

static uint64_t
inode_number_to_ref(
		const struct SqshPathResolver *resolver, uint32_t inode_number,
		int *err) {
	int rv = 0;
	uint64_t inode_ref = 0;
	const struct SqshInodeMap *inode_map = resolver->inode_map;

	inode_ref = sqsh_inode_map_get2(inode_map, inode_number, &rv);
	if (rv < 0) {
		goto out;
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	return inode_ref;
}

int
sqsh__path_resolver_init(
		struct SqshPathResolver *resolver, struct SqshArchive *archive) {
	int rv = 0;
	resolver->archive = archive;
	rv = sqsh_archive_inode_map(archive, &resolver->inode_map);
	if (rv < 0) {
		goto out;
	}
	const struct SqshConfig *config = sqsh_archive_config(archive);
	resolver->max_symlink_depth = SQSH_CONFIG_DEFAULT(
			config->max_symlink_depth, SQSH_DEFAULT_MAX_SYMLINKS_FOLLOWED);

	const struct SqshSuperblock *superblock =
			sqsh_archive_superblock(resolver->archive);
	resolver->root_inode_ref = sqsh_superblock_inode_root_ref(superblock);

out:
	return rv;
}

static struct SqshPathResolver *
new_resolver(struct SqshArchive *archive, int *err) {
	SQSH_NEW_IMPL(sqsh__path_resolver_init, struct SqshPathResolver, archive);
}

// TODO: To keep the API consistent, we need to move the resolver to the
// root_inode_ref in the public API. Internally we let the user take care of
// this.
struct SqshPathResolver *
sqsh_path_resolver_new(struct SqshArchive *archive, int *err) {
	int rv = 0;
	struct SqshPathResolver *resolver = new_resolver(archive, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_path_resolver_to_root(resolver);
	if (rv < 0) {
		goto out;
	}

out:
	if (err != NULL) {
		*err = rv;
	}
	if (rv < 0) {
		sqsh_path_resolver_free(resolver);
		return NULL;
	}
	return resolver;
}

int
sqsh__path_resolver_to_ref(
		struct SqshPathResolver *resolver, uint64_t inode_ref) {
	int rv = 0;
	rv = sqsh__file_cleanup(&resolver->cwd);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_init(&resolver->cwd, resolver->archive, inode_ref);
	if (rv < 0) {
		goto out;
	}

	if (sqsh_file_type(&resolver->cwd) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
		goto out;
	}

	rv = update_inode_from_cwd(resolver);
out:
	return rv;
}

int
sqsh__path_resolver_to_inode(
		struct SqshPathResolver *resolver, uint32_t inode_number) {
	int rv = 0;
	const uint64_t inode_ref = inode_number_to_ref(resolver, inode_number, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__path_resolver_to_ref(resolver, inode_ref);
out:
	return rv;
}

int
sqsh_path_resolver_to_root(struct SqshPathResolver *resolver) {
	return sqsh__path_resolver_to_ref(resolver, resolver->root_inode_ref);
}

int
sqsh_path_resolver_down(struct SqshPathResolver *resolver) {
	int rv = 0;
	if (is_beginning(resolver)) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_DOWN;
	} else if (sqsh__path_resolver_type(resolver) != SQSH_FILE_TYPE_DIRECTORY) {
		return -SQSH_ERROR_NOT_A_DIRECTORY;
	}
	const uint64_t child_inode_ref =
			sqsh_directory_iterator_inode_ref(&resolver->iterator);

	rv = sqsh__path_resolver_to_ref(resolver, child_inode_ref);

	if (rv == -SQSH_ERROR_NOT_A_DIRECTORY) {
		// We checked above if the directory iterator was pointing to a
		// directory if we get this error here, it means that either the inode
		// or the directory is corrupted.
		rv = -SQSH_ERROR_CORRUPTED_INODE;
	} else if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh_path_resolver_up(struct SqshPathResolver *resolver) {
	int rv = 0;
	if (!is_beginning(resolver)) {
		return update_inode_from_cwd(resolver);
	}

	if (sqsh_file_inode_ref(&resolver->cwd) == resolver->root_inode_ref) {
		return -SQSH_ERROR_WALKER_CANNOT_GO_UP;
	}
	const uint32_t parent_inode =
			sqsh_file_directory_parent_inode(&resolver->cwd);
	if (parent_inode <= 0) {
		rv = -SQSH_ERROR_CORRUPTED_INODE;
		goto out;
	}

	rv = sqsh__path_resolver_to_inode(resolver, parent_inode);
out:
	return rv;
}

int
sqsh_path_resolver_revert(struct SqshPathResolver *resolver) {
	return update_inode_from_cwd(resolver);
}

uint64_t
sqsh_path_resolver_inode_ref(const struct SqshPathResolver *resolver) {
	return resolver->current_inode_ref;
}

int
sqsh_path_resolver_lookup(
		struct SqshPathResolver *walker, const char *name,
		const size_t name_size) {
	int rv = 0;
	// revert to the beginning of the directory
	rv = update_inode_from_cwd(walker);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_directory_iterator_lookup(&walker->iterator, name, name_size);
	if (rv < 0) {
		goto out;
	}

	rv = update_inode_from_iterator(walker);
out:
	return rv;
}

uint32_t
sqsh_path_resolver_dir_inode(const struct SqshPathResolver *resolver) {
	if (is_beginning(resolver)) {
		return sqsh_file_directory_parent_inode(&resolver->cwd);
	} else {
		return sqsh_file_inode(&resolver->cwd);
	}
}

struct SqshFile *
sqsh_path_resolver_open_file(
		const struct SqshPathResolver *resolver, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	uint64_t inode_ref = resolver->current_inode_ref;
	file = sqsh_open_by_ref(resolver->archive, inode_ref, &rv);
	if (rv < 0) {
		goto out;
	}

	if (!is_beginning(resolver)) {
		uint64_t parent_inode_ref = sqsh_file_inode_ref(&resolver->cwd);
		sqsh__file_set_parent_inode_ref(file, parent_inode_ref);
	}
out:
	if (rv < 0) {
		sqsh__file_cleanup(file);
		file = NULL;
	}
	if (err != NULL) {
		*err = rv;
	}
	return file;
}

enum SqshFileType
sqsh__path_resolver_type(const struct SqshPathResolver *resolver) {
	if (is_beginning(resolver)) {
		return SQSH_FILE_TYPE_DIRECTORY;
	} else {
		return sqsh_directory_iterator_file_type(&resolver->iterator);
	}
}
int
sqsh__path_resolver_follow_symlink(struct SqshPathResolver *resolver) {
	int rv = 0;
	if (resolver->current_symlink_depth > resolver->max_symlink_depth) {
		return -SQSH_ERROR_TOO_MANY_SYMLINKS_FOLLOWED;
	} else if (sqsh__path_resolver_type(resolver) != SQSH_FILE_TYPE_SYMLINK) {
		return -SQSH_ERROR_NOT_A_SYMLINK;
	}
	resolver->current_symlink_depth++;

	const uint64_t inode_ref =
			sqsh_directory_iterator_inode_ref(&resolver->iterator);

	rv = update_inode_from_cwd(resolver);
	if (rv < 0) {
		goto out;
	}

	struct SqshFile file = {0};
	rv = sqsh__file_init(&file, resolver->archive, inode_ref);
	if (rv < 0) {
		goto out;
	}

	const char *target = sqsh_file_symlink(&file);
	size_t target_size = sqsh_file_symlink_size(&file);

	rv = path_resolve(resolver, target, target_size, true);

out:
	sqsh__file_cleanup(&file);
	return rv;
}

int
sqsh__path_resolver_follow_all_symlinks(struct SqshPathResolver *resolver) {
	while (1) {
		int rv = sqsh__path_resolver_follow_symlink(resolver);
		if (rv == -SQSH_ERROR_NOT_A_SYMLINK) {
			return 0;
		} else if (rv < 0) {
			return rv;
		}
	}
}

enum SqshFileType
sqsh_path_resolver_type(const struct SqshPathResolver *walker) {
	if (is_beginning(walker)) {
		return sqsh_file_type(&walker->cwd);
	} else {
		return sqsh_directory_iterator_file_type(&walker->iterator);
	}
}

static int
path_resolve(
		struct SqshPathResolver *resolver, const char *path, size_t path_len,
		bool follow_symlinks) {
	int rv = 0;
	const char *segment = path;
	const char *next_segment = NULL;
	size_t remaining_path_len = path_len;
	if (segment[0] == '/') {
		rv = sqsh_path_resolver_to_root(resolver);
		if (rv < 0) {
			goto out;
		}
	}

	bool is_dir = true;
	do {
		if (!is_dir) {
			rv = -SQSH_ERROR_NOT_A_DIRECTORY;
			goto out;
		}
		const size_t segment_len =
				path_segment_len(segment, remaining_path_len);
		next_segment = path_next_segment(segment, remaining_path_len);
		remaining_path_len -= segment_len + 1;
		if (segment_len == 0) {
			continue;
		} else if (segment_len == 1 && segment[0] == '.') {
			continue;
		} else if (segment_len == 2 && segment[0] == '.' && segment[1] == '.') {
			rv = sqsh_path_resolver_up(resolver);
			if (rv < 0) {
				goto out;
			}
			continue;
		} else {
			rv = sqsh_path_resolver_lookup(resolver, segment, segment_len);
			if (rv < 0) {
				goto out;
			}
		}

		if (next_segment != NULL || follow_symlinks) {
			rv = sqsh__path_resolver_follow_all_symlinks(resolver);
			if (rv < 0) {
				goto out;
			}
		}
		is_dir = sqsh_path_resolver_type(resolver) == SQSH_FILE_TYPE_DIRECTORY;
		if (is_dir && !is_beginning(resolver)) {
			rv = sqsh_path_resolver_down(resolver);
		}
	} while ((segment = next_segment) != NULL);
out:
	return rv;
}

int
sqsh__path_resolver_resolve_nt(
		struct SqshPathResolver *resolver, const char *path, size_t path_len,
		bool follow_symlinks) {
	resolver->current_symlink_depth = 0;
	return path_resolve(resolver, path, path_len, follow_symlinks);
}

int
sqsh_path_resolver_resolve(
		struct SqshPathResolver *resolver, const char *path,
		bool follow_symlinks) {
	return sqsh__path_resolver_resolve_nt(
			resolver, path, strlen(path), follow_symlinks);
}

int
sqsh__path_resolver_cleanup(struct SqshPathResolver *resolver) {
	sqsh__file_cleanup(&resolver->cwd);
	sqsh__directory_iterator_cleanup(&resolver->iterator);

	return 0;
}

int
sqsh_path_resolver_free(struct SqshPathResolver *resolver) {
	SQSH_FREE_IMPL(sqsh__path_resolver_cleanup, resolver);
}

/* TO BE DEPRECATED FUNCTIONS */

bool
sqsh_path_resolver_next(struct SqshPathResolver *walker, int *err) {
	struct SqshDirectoryIterator *iterator = &walker->iterator;
	int rv = 0;

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

const char *
sqsh_path_resolver_name(const struct SqshPathResolver *walker) {
	size_t size;
	if (is_beginning(walker)) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name2(&walker->iterator, &size);
	}
}

uint16_t
sqsh_path_resolver_name_size(const struct SqshPathResolver *walker) {
	size_t size;
	if (is_beginning(walker)) {
		return 0;
	} else {
		sqsh_directory_iterator_name2(&walker->iterator, &size);
		return (uint16_t)size;
	}
}

char *
sqsh_path_resolver_name_dup(const struct SqshPathResolver *walker) {
	if (is_beginning(walker)) {
		return NULL;
	} else {
		return sqsh_directory_iterator_name_dup(&walker->iterator);
	}
}
