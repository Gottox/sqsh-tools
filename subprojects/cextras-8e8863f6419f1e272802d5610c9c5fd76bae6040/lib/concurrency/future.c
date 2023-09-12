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

#include "../../include/cextras/concurrency.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

struct CxFuture {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	void *in_value;
	void *out_value;
};

struct CxFuture *
cx_future_init(void *in_value) {
	struct CxFuture *future = calloc(1, sizeof(struct CxFuture));
	if (future == NULL)
		return NULL;
	future->in_value = in_value;
	pthread_mutex_init(&future->mutex, NULL);
	pthread_cond_init(&future->cond, NULL);
	return future;
}

void *
cx_future_get_in_value(struct CxFuture *future) {
	return future->in_value;
}

void *
cx_future_wait(struct CxFuture *future) {
	void *out_value = NULL;
	pthread_mutex_lock(&future->mutex);
	while (future->out_value == NULL)
		pthread_cond_wait(&future->cond, &future->mutex);
	out_value = future->out_value;
	pthread_mutex_unlock(&future->mutex);
	return out_value;
}

int
cx_future_resolve(struct CxFuture *future, void *value) {
	int rv = 0;
	pthread_mutex_lock(&future->mutex);
	if (future->out_value != NULL) {
		rv = -1;
		goto out;
	}
	future->out_value = value;
	pthread_cond_broadcast(&future->cond);

out:
	pthread_mutex_unlock(&future->mutex);
	return rv;
}

int
cx_future_destroy(struct CxFuture *future) {
	pthread_mutex_destroy(&future->mutex);
	pthread_cond_destroy(&future->cond);
	free(future);
	return 0;
}
