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

#include "../../include/sqsh_extract_private.h"

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

#	define SQSH_LZO_MAX_WORKER 16

struct SqshLzoHelper {
	pid_t pid;
	FILE *compressed_fd;
	FILE *uncompressed_fd;
	pthread_mutex_t mutex;
};

pthread_once_t helper_initialized = PTHREAD_ONCE_INIT;
static size_t max_worker_count = 0;
static struct SqshLzoHelper helper[SQSH_LZO_MAX_WORKER] = {0};
static const char *helper_path = CONFIG_LZO_HELPER_PATH;

static void
spawn_helper_subprocess(struct SqshLzoHelper *helper) {
	int compressed_pipe[2];
	int uncompressed_pipe[2];
	posix_spawn_file_actions_t file_actions;

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

	const char *argv[] = {helper_path, "--internal\b", NULL};
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
	max_worker_count = get_nprocs();
	if (max_worker_count < 1) {
		max_worker_count = 1;
	} else if (max_worker_count > SQSH_LZO_MAX_WORKER) {
		max_worker_count = SQSH_LZO_MAX_WORKER;
	}

	const char *env_helper_path = secure_getenv("SQSH_LZO_HELPER_PATH");
	if (env_helper_path != NULL) {
		helper_path = env_helper_path;
	}

	for (sqsh_index_t i = 0; i < max_worker_count; i++) {
		if (pthread_mutex_init(&helper->mutex, NULL) != 0) {
			abort();
		}
	}
}

static int
sqsh_lzo_finish(void *context, uint8_t *target, size_t *target_size) {
	int rv = 0;
	if (pthread_once(&helper_initialized, init_helper) != 0) {
		rv = -SQSH_ERROR_TODO;
		return rv;
	}

	struct SqshLzoHelper *hlp = NULL;

	for (sqsh_index_t i = 0; i < max_worker_count; i++) {
		if (pthread_mutex_trylock(&helper[i].mutex) == 0) {
			hlp = &helper[i];
			break;
		}
	}
	if (hlp == NULL) {
		hlp = &helper[0];
		if (pthread_mutex_lock(&hlp->mutex) != 0) {
			rv = -SQSH_ERROR_TODO;
			goto out;
		}
	}

	if (hlp->pid == 0) {
		spawn_helper_subprocess(hlp);
	}

	const uint8_t *compressed = sqsh__extract_buffer_data(context);
	const uint64_t compressed_size = sqsh__extract_buffer_size(context);

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
	sqsh__extract_buffer_cleanup(context);
	return rv;
}

static const struct SqshExtractorImpl impl_lzo = {
		.init = sqsh__extract_buffer_init,
		.extract = sqsh__extract_buffer_decompress,
		.finish = sqsh_lzo_finish,
};

const struct SqshExtractorImpl *const sqsh__impl_lzo = &impl_lzo;
#else
const struct SqshExtractorImpl *const sqsh__impl_lzo = NULL;
#endif
