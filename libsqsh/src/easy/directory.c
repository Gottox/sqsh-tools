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

#include <stdint.h>
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

static int
directory_collector_next(void *iterator, const char **value, size_t *size) {
	int rv = 0;
	if (sqsh_directory_iterator_next(iterator, &rv)) {
		*value = sqsh_directory_iterator_name(iterator);
		*size = (size_t)sqsh_directory_iterator_name_size(iterator);
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
