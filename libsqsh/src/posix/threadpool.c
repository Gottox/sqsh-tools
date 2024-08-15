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
 * @file         thread_pool.c
 */

#define _DEFAULT_SOURCE

#include <sqsh_common_private.h>
#include <sqsh_error.h>
#include <sqsh_posix_private.h>

int
sqsh__threadpool_init(struct SqshThreadpool *pool, size_t threads) {
	return cx_threadpool_init(&pool->pool, threads);
}

struct SqshThreadpool *
sqsh_threadpool_new(size_t threads, int *err) {
	SQSH_NEW_IMPL(sqsh__threadpool_init, struct SqshThreadpool, threads);
}

int
sqsh__threadpool_cleanup(struct SqshThreadpool *pool) {
	return cx_threadpool_cleanup(&pool->pool);
}

int
sqsh_threadpool_wait(struct SqshThreadpool *pool) {
	return cx_threadpool_wait(&pool->pool);
}

int
sqsh_threadpool_free(struct SqshThreadpool *pool) {
	SQSH_FREE_IMPL(sqsh__threadpool_cleanup, pool);
}
