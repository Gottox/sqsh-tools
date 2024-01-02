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
 * @file         thread.c
 */

#define _DEFAULT_SOURCE

#include <sqsh_utils_private.h>

#include <sqsh_error.h>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

int
sqsh__mutex_init(sqsh__mutex_t *mutex) {
	int rv = pthread_mutex_init(mutex, NULL);
	if (rv != 0) {
		return -SQSH_ERROR_MUTEX_INIT_FAILED;
	}
	return 0;
}

int
sqsh__mutex_init_recursive(sqsh__mutex_t *mutex) {
	pthread_mutexattr_t mutex_attr;
	int rv = pthread_mutexattr_init(&mutex_attr);
	if (rv != 0) {
		return -SQSH_ERROR_MUTEX_INIT_FAILED;
	}

	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);

	rv = pthread_mutex_init(mutex, &mutex_attr);
	if (rv != 0) {
		rv = -SQSH_ERROR_MUTEX_INIT_FAILED;
		goto out;
	}

out:
	pthread_mutexattr_destroy(&mutex_attr);
	return rv;
}

int
sqsh__mutex_lock(sqsh__mutex_t *mutex) {
	int rv = pthread_mutex_lock(mutex);
	if (rv != 0) {
		return -SQSH_ERROR_MUTEX_LOCK_FAILED;
	} else {
		return 0;
	}
}

int
sqsh__mutex_unlock(sqsh__mutex_t *mutex) {
	int rv = pthread_mutex_unlock(mutex);
	if (rv != 0) {
		return -SQSH_ERROR_MUTEX_LOCK_FAILED;
	} else {
		return 0;
	}
}

int
sqsh__mutex_destroy(sqsh__mutex_t *mutex) {
	int rv = pthread_mutex_destroy(mutex);
	if (rv != 0) {
		return -SQSH_ERROR_MUTEX_DESTROY_FAILED;
	} else {
		return 0;
	}
}
