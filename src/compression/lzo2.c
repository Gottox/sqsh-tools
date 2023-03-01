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
 * @file         lzo.c
 */

#define _GNU_SOURCE

#include "../../include/sqsh_compression_private.h"

#include "../../include/sqsh_error.h"

#ifdef CONFIG_LZO

#	include <pthread.h>
#	include <unistd.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <limits.h>
#	include <fcntl.h>
#	include <spawn.h>
#	include <sys/sysinfo.h>

#	ifndef CONFIG_LZO_HELPER_PATH
#		error "CONFIG_LZO_HELPER_PATH not defined"
#	endif

#	define LZO_MAX_WORKER 16

struct SqshLzoHelper {
	pid_t pid;
	FILE *compressed_fd;
	FILE *uncompressed_fd;
	pthread_mutex_t mutex;
};

pthread_once_t helper_initialized = PTHREAD_ONCE_INIT;
static size_t worker_count = 0;
static sqsh_index_t next_worker = 0;
static struct SqshLzoHelper helper[LZO_MAX_WORKER] = {0};

static void
init_helper_subprocess(struct SqshLzoHelper *helper) {
	int compressed_pipe[2];
	int uncompressed_pipe[2];
	const char *helper_path = secure_getenv("SQSH_LZO_HELPER_PATH");
	posix_spawn_file_actions_t file_actions;

	if (pthread_mutex_init(&helper->mutex, NULL) != 0) {
		abort();
	}

	if (helper_path == NULL) {
		helper_path = CONFIG_LZO_HELPER_PATH;
	}

	if (pipe(compressed_pipe) == -1) {
		abort();
	}
	helper->compressed_fd = fdopen(compressed_pipe[1], "w");
	if (helper->compressed_fd == NULL) {
		perror("fdopen(compressed_pipe[1])");
		abort();
	}

	if (pipe(uncompressed_pipe) == -1) {
		abort();
	}
	helper->uncompressed_fd = fdopen(uncompressed_pipe[0], "r");
	if (helper->uncompressed_fd == NULL) {
		perror("fdopen(uncompressed_pipe[0])");
		abort();
	}

	fcntl(compressed_pipe[1], F_SETFD,
		  fcntl(compressed_pipe[1], F_GETFD) | FD_CLOEXEC);
	fcntl(uncompressed_pipe[0], F_SETFD,
		  fcntl(uncompressed_pipe[0], F_GETFD) | FD_CLOEXEC);

	posix_spawn_file_actions_init(&file_actions);

	posix_spawn_file_actions_adddup2(
			&file_actions, compressed_pipe[0], STDIN_FILENO);
	posix_spawn_file_actions_addclose(&file_actions, compressed_pipe[1]);

	posix_spawn_file_actions_adddup2(
			&file_actions, uncompressed_pipe[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&file_actions, uncompressed_pipe[0]);

	const char *argv[] = {helper_path, NULL};
	if (posix_spawn(
				&helper->pid, helper_path, &file_actions, NULL,
				(char *const *)argv, NULL) != 0) {
		perror(helper_path);
		abort();
	}

	close(compressed_pipe[0]);
	close(uncompressed_pipe[1]);

	posix_spawn_file_actions_destroy(&file_actions);
}

static void
init_helper(void) {
	worker_count = get_nprocs();
	if (worker_count < 1) {
		worker_count = 1;
	} else if (worker_count > LZO_MAX_WORKER) {
		worker_count = LZO_MAX_WORKER;
	}

	for (sqsh_index_t i = 0; i < worker_count; i++) {
		init_helper_subprocess(&helper[i]);
	}
}

static int
sqsh_lzo2_finish(void *context, uint8_t *target, size_t *target_size) {
	int rv = 0;
	if (pthread_once(&helper_initialized, init_helper) != 0) {
		rv = -SQSH_ERROR_TODO;
		return rv;
	}

	sqsh_index_t current_worker =
			__atomic_fetch_add(&next_worker, 1, __ATOMIC_SEQ_CST);
	current_worker %= worker_count;

	struct SqshLzoHelper *hlp = &helper[current_worker];

	if (pthread_mutex_lock(&hlp->mutex) != 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	const uint8_t *compressed = sqsh__buffering_compression_data(context);
	const uint64_t compressed_size = sqsh__buffering_compression_size(context);

	uint64_t target_size_64 = *target_size;
	rv = fwrite(&target_size_64, sizeof(uint64_t), 1, hlp->compressed_fd);
	if (rv != 1) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	rv = fwrite(&compressed_size, sizeof(uint64_t), 1, hlp->compressed_fd);
	if (rv != 1) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	rv = fwrite(
			compressed, sizeof(uint8_t), compressed_size, hlp->compressed_fd);
	if (rv < 0 || (uint64_t)rv != compressed_size) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	fflush(hlp->compressed_fd);

	uint64_t uncompressed_size = 0;
	int64_t remote_rv = 0;
	rv = fread(&remote_rv, sizeof(uint64_t), 1, hlp->uncompressed_fd);
	if (rv != 1 || remote_rv < 0) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	fread(&uncompressed_size, sizeof(uncompressed_size), 1,
		  hlp->uncompressed_fd);
	if (uncompressed_size > *target_size) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	rv = fread(target, 1, uncompressed_size, hlp->uncompressed_fd);
	if (rv > 0 && (uint64_t)rv != uncompressed_size) {
		rv = -SQSH_ERROR_TODO;
		goto out;
	}
	*target_size = uncompressed_size;

	pthread_mutex_unlock(&hlp->mutex);

out:
	sqsh__buffering_compression_cleanup(context);
	return rv;
}

static const struct SqshCompressionImpl impl = {
		.init = sqsh__buffering_compression_init,
		.decompress = sqsh__buffering_compression_decompress,
		.finish = sqsh_lzo2_finish,
};

const struct SqshCompressionImpl *sqsh__lzo2_impl = &impl;
#else
const struct SqshCompressionImpl *sqsh__lzo2_impl = NULL;
#endif
