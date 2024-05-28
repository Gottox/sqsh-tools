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
 * @file         sqsh_utils_private.h
 */

#ifndef SQSH_THREAD_PRIVATE_H
#define SQSH_THREAD_PRIVATE_H

#include <pthread.h>

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * utils/thread.c
 */

/**
 * @brief sqsh_thread_t represents a thread.
 */
typedef pthread_mutex_t sqsh__mutex_t;

/**
 * @brief sqsh__mutex_init initializes a mutex.
 *
 * @param mutex the mutex to initialize.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__mutex_init(sqsh__mutex_t *mutex);

/**
 * @brief sqsh__mutex_init initializes a mutex with support for
 * recursive locking.
 *
 * @param mutex the mutex to initialize.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int
sqsh__mutex_init_recursive(sqsh__mutex_t *mutex);

/**
 * @brief sqsh__mutex_lock locks a mutex.
 *
 * @param mutex the mutex to lock.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__mutex_lock(sqsh__mutex_t *mutex);

/**
 * @brief sqsh__mutex_lock unlocks a mutex.
 *
 * @param mutex the mutex to lock.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT int sqsh__mutex_unlock(sqsh__mutex_t *mutex);

/**
 * @brief sqsh__mutex_unlock unlocks a mutex.
 *
 * @param mutex the mutex to unlock.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT int sqsh__mutex_destroy(sqsh__mutex_t *mutex);

/***************************************
 * utils/math.c
 */

SQSH_NO_EXPORT unsigned long sqsh__log2(unsigned long x);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_THREAD_PRIVATE_H */
