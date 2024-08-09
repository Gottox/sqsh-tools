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
 * @file         posix.c
 */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <sqsh_archive.h>
#include <sqsh_error.h>
#include <sqsh_file_private.h>
#include <sqsh_posix.h>

#include <cextras/concurrency.h>

int
sqsh_file_to_stream(const struct SqshFile *file, FILE *stream) {
	int rv = 0;
	struct SqshFileIterator iterator = {0};

	rv = sqsh__file_iterator_init(&iterator, file);
	if (rv < 0) {
		goto out;
	}

	while (sqsh_file_iterator_next(&iterator, SIZE_MAX, &rv)) {
		const uint8_t *data = sqsh_file_iterator_data(&iterator);
		const size_t size = sqsh_file_iterator_size(&iterator);
		const size_t written = fwrite(data, sizeof(uint8_t), size, stream);
		if (written > 0 && written != size) {
			rv = -errno;
			goto out;
		}
	}

out:
	sqsh__file_iterator_cleanup(&iterator);
	return rv;
}

struct FileData {
	struct SqshFile file;
	sqsh__mutex_t mutex;
	bool owned_thread_pool;
	FILE *target_stream;
	struct CxThreadpool *thread_pool;
	sqsh_file_to_stream_mt_cb cb;
	void *userdata;
};

struct BlockData {
	uint64_t offset;
	struct FileData *file_data;
};

static void
extract_worker(void *user) {
	int rv = 0;
	struct BlockData *block_data = user;
	struct FileData *file_data = block_data->file_data;

	rv = sqsh__mutex_lock(&file_data->mutex);
	if (rv < 0) {
	}

	sqsh__mutex_unlock(&file_data->mutex);
}

static void
file_data_free(struct FileData *file_data) {
	if (file_data->owned_thread_pool) {
		cx_threadpool_destroy(file_data->thread_pool);
	}
	sqsh__file_cleanup(&file_data->file);
	fclose(file_data->target_stream);
	free(file_data);
}

static struct FileData *
file_data_new(
		const struct SqshFile *file, FILE *stream, void *thread_pool,
		sqsh_file_to_stream_mt_cb cb, void *userdata, int *err) {
	int rv = 0;
	struct FileData *file_data = calloc(1, sizeof(struct FileData));
	if (file_data == NULL) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}
	file_data->target_stream = stream;
	file_data->cb = cb;
	file_data->userdata = userdata;

	if (thread_pool != NULL) {
		file_data->owned_thread_pool = false;
		file_data->thread_pool = thread_pool;
	} else {
		file_data->owned_thread_pool = true;
		file_data->thread_pool = cx_threadpool_init(0);
		if (thread_pool == NULL) {
			rv = -SQSH_ERROR_INTERNAL;
			goto out;
		}
	}

	rv = sqsh__mutex_init(&file_data->mutex);
	if (rv < 0) {
		goto out;
	}
	struct SqshArchive *const archive = file->archive;
	const uint64_t inode_ref = sqsh_file_inode_ref(file);
	rv = sqsh__file_init(&file_data->file, archive, inode_ref);
	if (rv < 0) {
		goto out;
	}
out:
	*err = rv;
	if (rv < 0) {
		file_data_free(file_data);
		file_data = NULL;
	}
	return file_data;
}

static int
prepare_file(struct FileData *file_data) {
	const uint64_t file_size = sqsh_file_size(&file_data->file);
	int fd = fileno(file_data->target_stream);
	// Make sure the file is empty:
	if (ftruncate(fd, 0) < 0) {
		return -errno;
	}
	// Make sure the file is the right size:
	if (ftruncate(fd, (off_t)file_size) < 0) {
		return -errno;
	}

	return 0;
}

int
sqsh_file_to_stream_mt(
		const struct SqshFile *file, FILE *stream, void *thread_pool,
		sqsh_file_to_stream_mt_cb cb, void *data) {
	int rv = 0;
	struct FileData *file_data = NULL;
	struct SqshArchive *archive = file->archive;
	const uint32_t block_count = sqsh_file_block_count(file);
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	const uint64_t block_size = sqsh_superblock_block_size(superblock);
	struct BlockData *block_data =
			calloc(block_count, sizeof(struct SqshFile *));
	if (block_data == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	file_data = file_data_new(file, stream, thread_pool, cb, data, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = prepare_file(file_data);
	if (rv < 0) {
		goto out;
	}

	uint64_t block_offset = 0;
	for (uint64_t i = 0; i < block_count; i++) {
		block_data[i].offset = block_offset;
		block_data[i].file_data = file_data;
		rv = cx_threadpool_schedule(
				thread_pool, (uintptr_t)stream, extract_worker, &block_data[i]);
		if (rv < 0) {
			rv = -SQSH_ERROR_INTERNAL;
			goto out;
		}
		block_offset += block_size;
	}

	if (cb == NULL) {
		rv = cx_threadpool_wait(thread_pool, (uintptr_t)stream);
		if (rv < 0) {
			rv = -SQSH_ERROR_INTERNAL;
		}
		free(block_data);
		file_data_free(file_data);
	}

out:
	return rv;
}
