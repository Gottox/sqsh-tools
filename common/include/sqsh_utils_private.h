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

#ifndef SQSH_UTILS_PRIVATE_H
#define SQSH_UTILS_PRIVATE_H

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
 * @param locked set to true if the mutex was successfully locked.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int
sqsh__mutex_lock(sqsh__mutex_t *mutex, bool *locked);

/**
 * @brief sqsh__mutex_unlock unlocks a mutex if locked is true.
 *
 * @param mutex the mutex to unlock.
 * @param locked set to false after unlocking.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT int sqsh__mutex_unlock(sqsh__mutex_t *mutex, bool *locked);

/**
 * @brief sqsh__mutex_unlock unlocks a mutex.
 *
 * @param mutex the mutex to unlock.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_EXPORT int sqsh__mutex_destroy(sqsh__mutex_t *mutex);

/***************************************
 * utils/block.c
 */

/**
 * @brief Computes the number of full blocks that fit in @p value.
 *
 * Equivalent to `value / block_size` where `block_size = 1 << block_log`.
 *
 * @param value the value to divide.
 * @param block_log log2 of the block size.
 *
 * @return the number of full blocks.
 */
SQSH_NO_EXPORT uint64_t sqsh_block_count(uint64_t value, uint16_t block_log);

/**
 * @brief Computes the number of blocks needed to cover @p value, rounding up.
 *
 * Equivalent to `ceil(value / block_size)` where `block_size = 1 << block_log`.
 *
 * @param value the value to divide.
 * @param block_log log2 of the block size.
 *
 * @return the number of blocks, rounded up.
 */
SQSH_NO_EXPORT uint64_t
sqsh_block_count_ceil(uint64_t value, uint16_t block_log);

/**
 * @brief Computes the remainder of @p value within a block.
 *
 * Equivalent to `value % block_size` where `block_size = 1 << block_log`.
 *
 * @param value the value to compute the remainder for.
 * @param block_log log2 of the block size.
 *
 * @return the remainder.
 */
SQSH_NO_EXPORT uint64_t
sqsh_block_remainder(uint64_t value, uint16_t block_log);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_THREAD_PRIVATE_H */
