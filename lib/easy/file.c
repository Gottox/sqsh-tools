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
 * @file         file.c
 */

#define _DEFAULT_SOURCE

#include "../../include/sqsh_easy.h"

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_tree_private.h"

bool
sqsh_easy_file_exists(struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshTreeWalker walker = {0};
	bool exists = false;

	rv = sqsh__tree_walker_init(&walker, archive);
	if (rv == -SQSH_ERROR_NO_SUCH_FILE) {
		rv = 0;
		goto out;
	} else if (rv < 0) {
		goto out;
	}
	rv = sqsh_tree_walker_resolve(&walker, path, true);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__tree_walker_cleanup(&walker);
	if (err != NULL) {
		*err = rv;
	}
	return exists;
}

uint8_t *
sqsh_easy_file_content(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFileIterator iterator = {0};
	struct SqshFile *file = NULL;
	uint8_t *content = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_iterator_init(&iterator, file);
	if (rv < 0) {
		goto out;
	}

	size_t file_size = sqsh_file_size(file);
	content = calloc(file_size + 1, sizeof(*content));
	if (content == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	for (int pos = 0;
		 (rv = sqsh_file_iterator_next(&iterator, SIZE_MAX)) > 0;) {
		const uint8_t *data = sqsh_file_iterator_data(&iterator);
		const size_t size = sqsh_file_iterator_size(&iterator);
		memcpy(&content[pos], data, size);
		pos += size;
	}

out:
	sqsh__file_iterator_cleanup(&iterator);
	sqsh_close(file);
	if (rv < 0) {
		free(content);
		content = NULL;
	}
	if (err != NULL) {
		*err = rv;
	}
	return content;
}

size_t
sqsh_easy_file_size(struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	size_t file_size = 0;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	file_size = sqsh_file_size(file);

out:
	sqsh_close(file);
	if (err != NULL) {
		*err = rv;
	}
	return file_size;
}

mode_t
sqsh_easy_file_permission(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	mode_t permission = 0;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	permission = sqsh_file_permission(file);

out:
	sqsh_close(file);
	if (err != NULL) {
		*err = rv;
	}
	return permission;
}

time_t
sqsh_easy_file_mtime(struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	time_t modified = 0;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	modified = sqsh_file_modified_time(file);

out:
	sqsh_close(file);
	if (err != NULL) {
		*err = rv;
	}
	return modified;
}
