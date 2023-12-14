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

#include <sqsh_easy.h>

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include <sqsh_error.h>
#include <sqsh_file_private.h>
#include <sqsh_tree_private.h>

struct DirectoryIterator {
	struct SqshDirectoryIterator dir;
	const char *path;
	size_t path_size;
	struct CxBuffer value;
};

static int
directory_path_collector_next(
		void *iterator, const char **value, size_t *size) {
	struct DirectoryIterator *it = iterator;
	int rv = 0;
	if (sqsh_directory_iterator_next(&it->dir, &rv)) {
		cx_buffer_drain(&it->value);
		rv = cx_buffer_append(
				&it->value, (const uint8_t *)it->path, it->path_size);
		if (rv < 0) {
			goto out;
		}
		rv = cx_buffer_append(&it->value, (const uint8_t *)"/", 1);
		if (rv < 0) {
			goto out;
		}
		size_t name_size;
		const char *name = sqsh_directory_iterator_name2(&it->dir, &name_size);
		rv = cx_buffer_append(&it->value, (const uint8_t *)name, name_size);
		if (rv < 0) {
			goto out;
		}
		*value = (const char *)cx_buffer_data(&it->value);
		*size = cx_buffer_size(&it->value);
	}
out:
	return rv;
}

char **
sqsh_easy_directory_list_path(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	struct DirectoryIterator iterator = {0};
	char **list = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_init(&iterator.dir, file);
	if (rv < 0) {
		goto out;
	}

	rv = cx_buffer_init(&iterator.value);
	if (rv < 0) {
		goto out;
	}

	iterator.path = path;
	iterator.path_size = strlen(path);
	while (iterator.path_size > 0 &&
		   iterator.path[iterator.path_size - 1] == '/') {
		iterator.path_size--;
	}

	rv = cx_collect(&list, directory_path_collector_next, &iterator.dir);
	if (rv < 0) {
		goto out;
	}

out:
	cx_buffer_cleanup(&iterator.value);
	sqsh__directory_iterator_cleanup(&iterator.dir);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return list;
}

static int
directory_collector_next(void *iterator, const char **value, size_t *size) {
	int rv = 0;
	if (sqsh_directory_iterator_next(iterator, &rv)) {
		*value = sqsh_directory_iterator_name2(iterator, size);
	}
	return rv;
}

char **
sqsh_easy_directory_list(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	struct SqshDirectoryIterator iterator = {0};
	char **list = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__directory_iterator_init(&iterator, file);
	if (rv < 0) {
		goto out;
	}

	rv = cx_collect(&list, directory_collector_next, &iterator);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__directory_iterator_cleanup(&iterator);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return list;
}
