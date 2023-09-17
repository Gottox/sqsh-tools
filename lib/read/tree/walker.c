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

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "../../../include/sqsh_tree_private.h"

#include "../../../include/sqsh_archive_private.h"
#include "../../../include/sqsh_directory_private.h"
#include "../../../include/sqsh_error.h"
#include "../../../include/sqsh_file_private.h"
#include "../utils/utils.h"

#include <stdlib.h>
#include <string.h>

#define SQSH_DEFAULT_MAX_SYMLINKS_FOLLOWED 100

static int
tree_walker_init(struct SqshTreeWalker *walker, struct SqshArchive *archive) {
	return sqsh__path_resolver_init(&walker->inner, archive);
}

struct SqshTreeWalker *
sqsh_tree_walker_new(struct SqshArchive *archive, int *err) {
	SQSH_NEW_IMPL(tree_walker_init, struct SqshTreeWalker, archive);
}

int
sqsh_tree_walker_up(struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_up(&walker->inner);
}

int
sqsh_tree_walker_next(struct SqshTreeWalker *walker) {
	int rv = 0;
	bool has_next = sqsh_path_resolver_next(&walker->inner, &rv);
	if (rv < 0) {
		return rv;
	}
	return has_next ? 1 : 0;
}

enum SqshFileType
sqsh_tree_walker_type(const struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_type(&walker->inner);
}

const char *
sqsh_tree_walker_name(const struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_name(&walker->inner);
}

uint16_t
sqsh_tree_walker_name_size(const struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_name_size(&walker->inner);
}

char *
sqsh_tree_walker_name_dup(const struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_name_dup(&walker->inner);
}

int
sqsh_tree_walker_revert(struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_revert(&walker->inner);
}

int
sqsh_tree_walker_lookup(
		struct SqshTreeWalker *walker, const char *name,
		const size_t name_size) {
	return sqsh_path_resolver_lookup(&walker->inner, name, name_size);
}

int
sqsh_tree_walker_down(struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_down(&walker->inner);
}

int
sqsh_tree_walker_to_root(struct SqshTreeWalker *walker) {
	return sqsh_path_resolver_to_root(&walker->inner);
}

struct SqshFile *
sqsh_tree_walker_open_file(const struct SqshTreeWalker *walker, int *err) {
	return sqsh_path_resolver_open_file(&walker->inner, err);
}

int
sqsh_tree_walker_resolve(
		struct SqshTreeWalker *walker, const char *path, bool follow_links) {
	return sqsh_path_resolver_resolve(&walker->inner, path, follow_links);
}

static int
tree_walker_cleanup(struct SqshTreeWalker *walker) {
	return sqsh__path_resolver_cleanup(&walker->inner);
}

int
sqsh_tree_walker_free(struct SqshTreeWalker *walker) {
	SQSH_FREE_IMPL(tree_walker_cleanup, walker);
}
