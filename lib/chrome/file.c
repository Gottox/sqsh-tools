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

#include <pthread.h>
#include <stdatomic.h>
#include <sys/sysinfo.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_inode.h"

#if 0
struct SqshToStreamThreadInfo {
	pthread_t thread;
	pthread_barrier_t *barrier;
	struct SqshFileIterator iterator[2];
	size_t worker_count;
	bool finished;
	bool joining;
};

void *
to_stream_worker(void *arg) {
	int rv = 0;
	struct SqshToStreamThreadInfo *worker = arg;
	int current = 0;

	do {
		struct SqshFileIterator *iterator = &worker->iterator[current];
		pthread_barrier_t *barrier = &worker->barrier[current];
		if (worker->finished == false) {
			rv = sqsh_file_iterator_next(iterator, 1);
			if (rv < 0) {
				goto out;
			} else if (rv == 0) {
				worker->finished = true;
			}
		}
		current = (current + 1) % 2;
		pthread_barrier_wait(barrier);
	} while (worker->joining == false);

out:
	return NULL;
}
int
sqsh_file_to_stream_mt(const struct SqshInode *inode, FILE *file) {
	int rv = 0;
	int nprocs = get_nprocs();
	if (nprocs < 0) {
		return -SQSH_ERROR_TODO;
	}
	pthread_barrier_t barrier[2];
	size_t worker_count = nprocs - 1;
	struct SqshToStreamThreadInfo workers[worker_count];

	for (sqsh_index_t i = 0; i < 2; i++) {
		rv = pthread_barrier_init(
				&barrier[i], NULL, worker_count + 1);
		if (rv < 0) {
			goto out;
		}
	}

	for (sqsh_index_t i = 0; i < worker_count; i++) {
		struct SqshToStreamThreadInfo *worker = &workers[i];
		worker->worker_count = worker_count;
		worker->barrier = workers[i].barrier;
		worker->finished = false;
		worker->joining = false;

		for (sqsh_index_t j = 0; j < worker_count; j++) {
			rv = sqsh__file_iterator_init(&worker->iterator[j], inode);
			if (rv < 0) {
				goto out;
			}
			rv = sqsh_file_iterator_skip(&worker->iterator[j], i + (j * worker_count));
			if (rv < 0) {
				goto out;
			}
		}

		rv = pthread_create(&worker->thread, NULL, to_stream_worker, worker);
		if (rv < 0) {
			goto out;
		}
	}

	for (sqsh_index_t finished = 0; finished < worker_count;) {
		for (sqsh_index_t i = 0; i < 2; i++) {
			pthread_barrier_wait(&barrier[i]);
			for (sqsh_index_t j = 0; j < worker_count; j++) {
				if (workers[j].joining) {
					continue;
				} else if (workers[j].finished) {
					finished++;
					pthread_join(workers[j].thread, NULL);
				}
			}
		}
	}
out:
	return 0;
}
#endif

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
		const size_t size = rv;
		rv = fwrite(data, sizeof(uint8_t), size, file);
		if (rv > 0 && (size_t)rv != size) {
			rv = -SQSH_ERROR_TODO;
			goto out;
		}
	}

out:
	sqsh__file_iterator_cleanup(&iterator);
	return rv;
}
