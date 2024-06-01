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
 * @file         directory.c
 */

#include "sqsh_tree.h"
#define _DEFAULT_SOURCE

#include <sqsh_easy.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sqsh_error.h>
#include <sqsh_tree_private.h>

struct TreeTraversalIterator {
	struct SqshTreeTraversal traversal;
	char *value;
};

static int
tree_traversal_collector_next(
		void *iterator, const char **value, size_t *size) {
	struct TreeTraversalIterator *it = iterator;
	int rv = 0;
	free(it->value);

	while (sqsh_tree_traversal_next(&it->traversal, &rv)) {
		enum SqshTreeTraversalState state =
				sqsh_tree_traversal_state(&it->traversal);
		if (state != SQSH_TREE_TRAVERSAL_STATE_FILE &&
			state != SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN) {
			continue;
		}

		it->value = sqsh_tree_traversal_path_dup(&it->traversal);
		if (it->value == NULL) {
			rv = -SQSH_ERROR_MALLOC_FAILED;
			goto out;
		}
		*value = it->value;
		*size = strlen(*value);
		break;
	}

out:
	return rv;
}

char **
sqsh_easy_tree_traversal(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	struct TreeTraversalIterator iterator = {0};
	char **list = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	if (sqsh_file_type(file) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
		goto out;
	}

	rv = sqsh__tree_traversal_init(&iterator.traversal, file);
	if (rv < 0) {
		goto out;
	}

	// Drop the root path
	bool has_next = sqsh_tree_traversal_next(&iterator.traversal, &rv);
	if (has_next == false) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	rv = cx_collect(&list, tree_traversal_collector_next, &iterator);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__tree_traversal_cleanup(&iterator.traversal);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return list;
}
