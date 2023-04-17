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
 * @file         sqsh_thread_private.h
 */

#ifndef SQSH_THREAD_PRIVATE_H
#define SQSH_THREAD_PRIVATE_H

#include <pthread.h>

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_mutex_t sqsh_mutex_t;

/**
 * @brief sqsh_mutex_init initializes a mutex.
 *
 * @param mutex the mutex to initialize.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_mutex_init(sqsh_mutex_t *mutex);

/**
 * @brief sqsh_mutex_init initializes a mutex with support for
 * recursive locking.
 *
 * @param mutex the mutex to initialize.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_mutex_init_recursive(sqsh_mutex_t *mutex);

/**
 * @brief sqsh_mutex_lock locks a mutex.
 *
 * @param mutex the mutex to lock.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_mutex_lock(sqsh_mutex_t *mutex);

/**
 * @brief sqsh_mutex_trylock tries to lock a mutex.
 * If the mutex is already locked, the function returns immediately.
 *
 * @param mutex the mutex to lock.
 *
 * @return 0 on success, less than 0 on error, 1 if the mutex is already locked.
 */
SQSH_NO_UNUSED int sqsh_mutex_trylock(sqsh_mutex_t *mutex);

/**
 * @brief sqsh_mutex_lock unlocks a mutex.
 *
 * @param mutex the mutex to lock.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_mutex_unlock(sqsh_mutex_t *mutex);

/**
 * @brief sqsh_mutex_unlock unlocks a mutex.
 *
 * @param mutex the mutex to unlock.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_mutex_destroy(sqsh_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
#endif // SQSH_THREAD_PRIVATE_H
