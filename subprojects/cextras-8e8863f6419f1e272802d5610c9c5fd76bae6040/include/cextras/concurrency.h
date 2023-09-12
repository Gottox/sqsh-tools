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
 * @file         threadpool.h
 */

#ifndef THREADPOOL_H

#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/***************************************
 * concurrency/threadpool.c
 */

/**
 * @brief A function running a thread pool task.
 */
typedef struct CxThreadpool *cx_threadpool_t;

/**
 * @brief A function running a thread pool task.
 */
typedef void (*cx_threadpool_task_t)(void *);

/**
 * @brief Initializes a threadpool.
 */
cx_threadpool_t cx_threadpool_init(size_t num_threads);

/**
 * @brief Adds a task to the threadpool.
 *
 * @param threadpool The threadpool to add the task to.
 * @param task The task function to run the task
 * @param arg The argument to the task function.
 */
int cx_threadpool_schedule(
		cx_threadpool_t threadpool, uintptr_t group, cx_threadpool_task_t task,
		void *arg);

/**
 * @brief Cleans up a threadpool.
 */
int cx_threadpool_destroy(cx_threadpool_t threadpool);

/***************************************
 * concurrency/future.c
 */

typedef struct CxFuture *cx_future_t;

/**
 * @brief Initializes a future.
 *
 * @param in_value The value to store in the future.
 *
 * @return The initialized future.
 */
cx_future_t cx_future_init(void *in_value);

/**
 * @brief Gets the value of a future. If the future is not ready, this function
 * blocks.
 *
 * @param future The future to get the value from.
 *
 * @return The value of the future.
 */
void *cx_future_get_in_value(cx_future_t future);

/**
 * @brief Gets the value of a future. If the future is not ready, this function
 * blocks.
 *
 * @param future The future to get the value from.
 *
 * @return The value of the future.
 */
void *cx_future_wait(cx_future_t future);

/**
 * @brief resolves a future.
 *
 * @param future The future to resolve.
 *
 * @return 0 on success, -1 on error.
 */
int cx_future_resolve(cx_future_t future, void *value);

/**
 * @brief Cleans up a future.
 *
 * @param future The future to clean up.
 *
 * @return 0 on success, -1 on error.
 */
int cx_future_destroy(cx_future_t future);

#ifdef __cplusplus
}
#endif
#endif // THREADPOOL_H
