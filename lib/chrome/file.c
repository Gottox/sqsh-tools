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

#include "../../include/sqsh_chrome.h"

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_inode.h"
#include "../../include/sqsh_tree_private.h"

bool
sqsh_file_exists(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	bool exists = false;

	inode = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	exists = true;

out:
	sqsh_close(inode);
	return exists;
}

char *
sqsh_file_content(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshFileIterator iterator = {0};
	struct SqshInode *inode = NULL;
	char *content = NULL;

	inode = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_iterator_init(&iterator, inode);
	if (rv < 0) {
		goto out;
	}

	size_t file_size = sqsh_inode_file_size(inode);
	content = calloc(file_size + 1, sizeof(char));
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
	sqsh_close(inode);
	if (rv < 0) {
		free(content);
		content = NULL;
	}
	return content;
}

size_t
sqsh_file_size(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	size_t file_size = 0;

	inode = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	file_size = sqsh_inode_file_size(inode);

out:
	sqsh_close(inode);
	return file_size;
}

mode_t
sqsh_file_permission(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	mode_t permission = 0;

	inode = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	permission = sqsh_inode_permission(inode);

out:
	sqsh_close(inode);
	return permission;
}

time_t
sqsh_file_mtime(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	time_t modified = 0;

	inode = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	modified = sqsh_inode_modified_time(inode);

out:
	sqsh_close(inode);
	return modified;
}
