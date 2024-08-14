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
 * @file         sqsh_posix.h
 */

#ifndef SQSH_POSIX_H
#define SQSH_POSIX_H

#include "sqsh_common.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SqshFile;
struct SqshFileIterator;

struct SqshThreadpool;

typedef void (*sqsh_file_iterator_mt_cb)(
		const struct SqshFile *file, const struct SqshFileIterator *iterator,
		uint64_t offset, void *data, int err);
typedef void (*sqsh_file_to_stream_mt_cb)(
		const struct SqshFile *file, FILE *stream, void *data, int err);

/**
 * @memberof SqshFile
 * @brief writes data to a file descriptor.
 *
 * @param[in] file The file context.
 * @param[in] stream The descriptor to write the file contents to.
 *
 * @return The number of bytes read on success, less than 0 on error.
 */
int sqsh_file_to_stream(const struct SqshFile *file, FILE *stream);

/**
 * @memberof SqshFile
 * @brief writes data to a file descriptor.
 *
 * @param[in] file The file context.
 * @param[in] threadpool The threadpool to use.
 * @param[in] stream The descriptor to write the file contents to.
 * @param[in] cb The callback to call when the operation is done.
 * @param[in] data The data to pass to the callback.
 */
void sqsh_file_to_stream_mt(
		const struct SqshFile *file, struct SqshThreadpool *threadpool,
		FILE *stream, sqsh_file_to_stream_mt_cb cb, void *data);

struct SqshThreadpool *sqsh_threadpool_new(size_t threads, int *err);

int sqsh_threadpool_wait(struct SqshThreadpool *pool);

/**
 * @memberof SqshThreadpool
 * @brief cleans up a threadpool.
 *
 * @param[in] pool The threadpool to uclean.
 * @return The threadpool on success, NULL on error.
 */
int sqsh__threadpool_cleanup(struct SqshThreadpool *pool);

/**
 * @memberof SqshThreadpool
 * @brief creates a new threadpool.
 *
 * @param[in] pool The threadpool to free.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_threadpool_free(struct SqshThreadpool *pool);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_POSIX_H */
