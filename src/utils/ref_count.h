/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : refcount
 * @created     : Saturday Dec 18, 2021 22:42:47 CET
 */

#include "../utils.h"
#include <stddef.h>

#ifndef REFCOUNT_H

#define REFCOUNT_H

typedef int (*hsqsRefCountDtor)(void *);

struct HsqsRefCount {
	size_t references;
	hsqsRefCountDtor dtor;
};

int hsqs_ref_count_new(
		struct HsqsRefCount **ref_count, size_t object_size,
		hsqsRefCountDtor dtor);

void *hsqs_ref_count_retain(struct HsqsRefCount *ref_count);

int hsqs_ref_count_release(struct HsqsRefCount *ref_count);

#endif /* end of include guard REFCOUNT_H */
