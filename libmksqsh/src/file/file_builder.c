/******************************************************************************
 *                                                                            *
 * Copyright (c) 2024, Enno Boland <g@s01.de>                                 *
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
 * @file         simple_archive.c
 *
 * This file implements a very small SquashFS writer that is only capable of
 * creating archives that contain a handful of files in the root directory.
 * The implementation is deliberately tiny and only implements what is needed
 * for the unit tests in this repository.  It should not be used as a full
 * featured replacement for mksquashfs.
 */

#define _DEFAULT_SOURCE

#include <mksqsh_archive_private.h>
#include <mksqsh_metablock.h>

#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <cextras/memory.h>
#include <endian.h>
#include <mksqsh_file_private.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct MksqshFile *
mksqsh_file_retain(struct MksqshFile *file) {
	if (file != NULL) {
		cx_rc_retain(&file->rc);
	}
	return file;
}

struct MksqshFile *
mksqsh_file_new(
		struct MksqshArchive *archive, enum MksqshFileType type, int *err) {
	struct CxPreallocPool *pool = mksqsh__archive_file_pool(archive);
	struct MksqshFile *file = cx_prealloc_pool_get(pool);
	int rv = 0;
	if (file == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
	} else {
		memset(file, 0, sizeof(*file));
		cx_rc_init(&file->rc);
		file->type = type;
		file->archive = archive;
		file->content_type = MKSQSH_FILE_CONTENT_NONE;
	}
	if (err != NULL) {
		*err = rv;
	}
	return rv < 0 ? NULL : file;
}

int
mksqsh_file_add(
		struct MksqshFile *directory, const char *file_name,
		struct MksqshFile *child) {
	if (directory->type != MKSQSH_FILE_TYPE_DIR) {
		return -SQSH_ERROR_INTERNAL;
	}

	char *dup = strdup(file_name);
	if (dup == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}

	struct MksqshFile **tmp = reallocarray(
			directory->children, directory->child_count + 1, sizeof(*tmp));
	if (tmp == NULL) {
		free(dup);
		return -SQSH_ERROR_MALLOC_FAILED;
	}

	free(child->name);
	child->name = dup;
	directory->children = tmp;
	directory->children[directory->child_count++] = mksqsh_file_retain(child);
	return 0;
}

static void
mksqsh_file_reset_content(struct MksqshFile *file) {
	free(file->data);
	file->data = NULL;
	file->size = 0;
	if (file->content_type == MKSQSH_FILE_CONTENT_FD && file->fd != NULL) {
		fclose(file->fd);
	}
	file->fd = NULL;
	free(file->path);
	file->path = NULL;
	file->content_type = MKSQSH_FILE_CONTENT_NONE;
}

void
mksqsh_file_content_from_fd(struct MksqshFile *file, FILE *source, int *err) {
	int rv = 0;
	if (file->type != MKSQSH_FILE_TYPE_REG || source == NULL) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}
	mksqsh_file_reset_content(file);
	file->fd = source;
	file->content_type = MKSQSH_FILE_CONTENT_FD;
out:
	if (err != NULL) {
		*err = rv;
	}
}

void
mksqsh_file_content_from_path(
		struct MksqshFile *file, const char *path, int *err) {
	int rv = 0;
	if (file->type != MKSQSH_FILE_TYPE_REG || path == NULL) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}
	char *dup = strdup(path);
	if (dup == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	mksqsh_file_reset_content(file);
	file->path = dup;
	file->content_type = MKSQSH_FILE_CONTENT_PATH;
out:
	if (err != NULL) {
		*err = rv;
	}
}

void
mksqsh_file_content(struct MksqshFile *file, const char *content, size_t size) {
	if (file->type != MKSQSH_FILE_TYPE_REG) {
		return;
	}
	mksqsh_file_reset_content(file);
	uint8_t *dup = NULL;
	if (size > 0) {
		dup = calloc(size, 1);
		if (dup == NULL) {
			return;
		}
		memcpy(dup, content, size);
	}
	file->data = dup;
	file->size = size;
	file->content_type = MKSQSH_FILE_CONTENT_MEMORY;
}

void
mksqsh_file_release(struct MksqshFile *file) {
	if (!cx_rc_release(&file->rc)) {
		return;
	}
	for (size_t i = 0; i < file->child_count; i++) {
		mksqsh_file_release(file->children[i]);
	}
	free(file->children);
	mksqsh_file_reset_content(file);
	free(file->name);
	struct CxPreallocPool *pool = mksqsh__archive_file_pool(file->archive);
	cx_prealloc_pool_recycle(pool, file);
}
