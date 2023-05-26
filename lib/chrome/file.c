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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/sysinfo.h>
#include <errno.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_inode.h"
#include "../../include/sqsh_tree_private.h"

int
sqsh_file_to_stream(const struct SqshInode *inode, FILE *file) {
	int rv = 0;
	struct SqshFileIterator iterator = {0};

	rv = sqsh__file_iterator_init(&iterator, inode);
	if (rv < 0) {
		goto out;
	}

	while ((rv = sqsh_file_iterator_next(&iterator, SIZE_MAX)) > 0) {
		const uint8_t *data = sqsh_file_iterator_data(&iterator);
		const size_t size = sqsh_file_iterator_size(&iterator);
		rv = fwrite(data, sizeof(uint8_t), size, file);
		if (rv > 0 && (size_t)rv != size) {
			rv = -errno;
			goto out;
		}
	}

out:
	sqsh__file_iterator_cleanup(&iterator);
	return rv;
}


static int
file_open_path(struct SqshInode *inode, struct SqshArchive *archive, const char *path) {
	int rv;
	struct SqshTreeWalker walker = {0};

	rv = sqsh__tree_walker_init(&walker, archive);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_tree_walker_resolve(&walker, path, true);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__inode_init(inode, archive, walker.current_inode_ref);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh__tree_walker_cleanup(&walker);
	return rv;
}

bool
sqsh_file_exists(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode inode = {0};
	bool exists = false;

	rv = file_open_path(&inode, archive, path);
	if (rv < 0) {
		goto out;
	}

	exists = true;

out:
	sqsh__inode_cleanup(&inode);
	return exists;
}

char *
sqsh_file_content(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshFileIterator iterator = {0};
	struct SqshInode inode = {0};

	rv = file_open_path(&inode, archive, path);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_iterator_init(&iterator, &inode);
	if (rv < 0) {
		goto out;
	}

	size_t file_size = sqsh_inode_file_size(&inode);
	char *content = calloc(file_size + 1, sizeof(char));
	if (content == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	for (int pos = 0;
		 (rv = sqsh_file_iterator_next(&iterator, SIZE_MAX)) > 0;) {
		const uint8_t *data = sqsh_file_iterator_data(&iterator);
		const size_t size = sqsh_file_iterator_size(&iterator);
		memcpy(content + pos, data, size);
		pos += size;
	}

out:
	sqsh__file_iterator_cleanup(&iterator);
	sqsh__inode_cleanup(&inode);
	if (rv < 0) {
		free(content);
		content = NULL;
	}
	return content;
}

size_t
sqsh_file_size(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode inode = {0};
	size_t file_size = 0;

	rv = file_open_path(&inode, archive, path);
	if (rv < 0) {
		goto out;
	}

	file_size = sqsh_inode_file_size(&inode);

out:
	sqsh__inode_cleanup(&inode);
	return file_size;
}

mode_t
sqsh_file_permission(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode inode = {0};
	size_t permission = 0;

	rv = file_open_path(&inode, archive, path);
	if (rv < 0) {
		goto out;
	}

	permission = sqsh_inode_permission(&inode);

out:
	sqsh__inode_cleanup(&inode);
	return permission;
}

time_t
sqsh_file_mtime(struct SqshArchive *archive, const char *path) {
	int rv = 0;
	struct SqshInode inode = {0};
	size_t modified = 0;

	rv = file_open_path(&inode, archive, path);
	if (rv < 0) {
		goto out;
	}

	modified = sqsh_inode_modified_time(&inode);

out:
	sqsh__inode_cleanup(&inode);
	return modified;
}
