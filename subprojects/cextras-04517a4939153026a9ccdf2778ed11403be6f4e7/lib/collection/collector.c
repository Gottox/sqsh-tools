/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : iterator_collector
 * @created     : Tuesday Aug 29, 2023 17:27:35 CEST
 */

#include "../../include/cextras/collection.h"
#include "../../include/cextras/error.h"

int
cx_collect(char ***target, cx_collector_next_t next, void *iterator) {
	int rv = 0;
	static const uintptr_t nullptr = 0;
	struct CxBuffer list = {0};
	struct CxBuffer list_values = {0};
	size_t elements = 0;
	char **result = NULL;

	rv = cx_buffer_init(&list);
	if (rv < 0) {
		goto out;
	}
	rv = cx_buffer_init(&list_values);
	if (rv < 0) {
		goto out;
	}

	for (;;) {
		const char *value = NULL;
		size_t size = 0;

		rv = next(iterator, &value, &size);
		if (rv < 0) {
			goto out;
		}
		if (value == NULL) {
			break;
		}

		size_t index = cx_buffer_size(&list_values);
		char *index_ptr = (void *)index;
		elements++;
		rv = cx_buffer_append(&list, (uint8_t *)&index_ptr, sizeof(char *));
		if (rv < 0) {
			goto out;
		}
		rv = cx_buffer_append(&list_values, (uint8_t *)value, size);
		if (rv < 0) {
			goto out;
		}
		rv = cx_buffer_append(&list_values, (uint8_t *)&nullptr, sizeof(char));
		if (rv < 0) {
			goto out;
		}
	}

	rv = cx_buffer_append(&list, (uint8_t *)&nullptr, sizeof(char *));
	size_t base_size = cx_buffer_size(&list);

	const uint8_t *values_data = cx_buffer_data(&list_values);
	size_t values_size = cx_buffer_size(&list_values);

	rv = cx_buffer_append(&list, values_data, values_size);
	if (rv < 0) {
		goto out;
	}

	result = (char **)cx_buffer_unwrap(&list);
	for (cx_index_t i = 0; i < elements; i++) {
		result[i] += base_size + (uintptr_t)result;
	}
	*target = result;

out:
	cx_buffer_cleanup(&list);
	cx_buffer_cleanup(&list_values);
	return rv;
}
