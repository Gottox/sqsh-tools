/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : refcount
 * @created     : Saturday Dec 18, 2021 22:49:36 CET
 */

#include "ref_count.h"
#include "../error.h"
#include "../utils.h"

#include <stdlib.h>

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
	return (void *)&ref_count[1];
}

int
hsqs_ref_count_release(struct HsqsRefCount *ref_count) {
	ref_count->references--;
	if (ref_count->references == 0) {
		ref_count->dtor(hsqs_ref_count_retain(ref_count));
		free(ref_count);
	}
	return ref_count->references;
}
