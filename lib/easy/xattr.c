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
 * @file         xattr.c
 */

#define _DEFAULT_SOURCE

#include "../../include/sqsh_easy.h"

#include <cextras/collection.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_tree_private.h"
#include "../../include/sqsh_xattr_private.h"

char *
sqsh_easy_xattr_get(
		struct SqshArchive *archive, const char *path, const char *key,
		int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	struct SqshXattrIterator iterator = {0};
	char *xattr_value = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__xattr_iterator_init(&iterator, file);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_xattr_iterator_lookup(&iterator, key);
	if (rv < 0) {
		goto out;
	}

	xattr_value = sqsh_xattr_iterator_value_dup(&iterator);

out:
	sqsh__xattr_iterator_cleanup(&iterator);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return xattr_value;
}

struct XattrEasyIter {
	struct SqshXattrIterator iterator;
	char *key_buffer;
};

static int
xattr_collector_next(void *iterator, const char **value, size_t *size) {
	struct XattrEasyIter *i = iterator;
	int rv = 0;
	free(i->key_buffer);
	if (sqsh_xattr_iterator_next(&i->iterator, &rv)) {
		i->key_buffer = sqsh_xattr_iterator_fullname_dup(&i->iterator);

		*value = i->key_buffer;
		*size = (size_t)strlen(i->key_buffer);
	}
	return rv;
}

char **
sqsh_easy_xattr_keys(struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	struct XattrEasyIter iterator = {0};
	char **list = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__xattr_iterator_init(&iterator.iterator, file);
	if (rv < 0) {
		goto out;
	}

	rv = cx_collect(&list, xattr_collector_next, &iterator);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__xattr_iterator_cleanup(&iterator.iterator);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return list;
}
