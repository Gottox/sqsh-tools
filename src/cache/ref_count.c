/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : refcount
 * @created     : Saturday Dec 18, 2021 22:49:36 CET
 */

#include "ref_count.h"
#include "../error.h"
#include "../utils.h"

#include <stdlib.h>

static void *
get_data(struct HsqsRefCount *ref_count) {
	return (void *)&ref_count[1];
}
int
hsqs_ref_count_new(
		struct HsqsRefCount **ref_count, size_t object_size,
		hsqsRefCountDtor dtor) {
	struct HsqsRefCount *tmp;
	size_t outer_size = 0;
	if (ADD_OVERFLOW(sizeof(struct HsqsRefCount), object_size, &outer_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	tmp = calloc(1, outer_size);
	if (tmp == NULL) {
		return -HSQS_ERROR_MALLOC_FAILED;
	}

	tmp->dtor = dtor;
	tmp->references = 0;
	*ref_count = tmp;
	return 0;
}

void *
hsqs_ref_count_retain(struct HsqsRefCount *ref_count) {
	ref_count->references++;
	return get_data(ref_count);
}

int
hsqs_ref_count_release(struct HsqsRefCount *ref_count) {
	if (ref_count == NULL) {
		return 0;
	}

	ref_count->references--;
	if (ref_count->references == 0) {
		ref_count->dtor(get_data(ref_count));
		free(ref_count);
		return 0;
	} else {
		return ref_count->references;
	}
}
