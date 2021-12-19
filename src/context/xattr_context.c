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
 * @file        : xattr_context
 * @created     : Sunday Dec 19, 2021 22:19:33 CET
 */

#include "xattr_context.h"

int
hsqs_xattr_table_iterator_init(
		struct HsqsXattrTableIterator *iterator,
		struct HsqsXattrTable *xattr_table,
		const struct HsqsInodeContext *inode) {
	int rv;
	struct HsqsXattrLookupTable ref = {0};
	uint32_t index = hsqs_inode_xattr_index(inode);

	if (index == HSQS_INODE_NO_XATTR) {
		iterator->remaining_entries = 0;
		return 0;
	}

	rv = hsqs_table_get(&xattr_table->table, index, &ref);
	if (rv < 0) {
		hsqs_xattr_table_iterator_cleanup(iterator);
		goto out;
	}

	const struct HsqsXattrIdTable *header = get_header(xattr_table);
	// TODO: references are used in inodes too. Generalize!
	uint64_t start_block = hsqs_data_xattr_id_table_xattr_table_start(header);

	// TODO upper bounds should not be ~0.
	rv = hsqs_metablock_stream_init(
			&iterator->metablock, xattr_table->hsqs, start_block, ~0);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_metablock_stream_seek_ref(
			&iterator->metablock, hsqs_data_xattr_lookup_table_xattr_ref(&ref));
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_metablock_stream_more(
			&iterator->metablock, hsqs_data_xattr_lookup_table_size(&ref));
	if (rv < 0) {
		goto out;
	}

	iterator->remaining_entries = hsqs_data_xattr_lookup_table_count(&ref);
	iterator->next_offset = 0;
	iterator->key_offset = 0;
	iterator->value_offset = 0;
	iterator->context = xattr_table;

out:
	if (rv < 0) {
		hsqs_xattr_table_iterator_cleanup(iterator);
	}
	return rv;
}

static const struct HsqsXattrValue *
get_value(struct HsqsXattrTableIterator *iterator) {
	const uint8_t *data;
	if (iterator->value_offset == 0) {
		data = hsqs_metablock_stream_data(&iterator->out_of_line_value);
	} else {
		data = hsqs_metablock_stream_data(&iterator->metablock);
	}
	return (const struct HsqsXattrValue *)&data[iterator->value_offset];
}

static const struct HsqsXattrKey *
get_key(struct HsqsXattrTableIterator *iterator) {
	const uint8_t *data = hsqs_metablock_stream_data(&iterator->metablock);

	return (const struct HsqsXattrKey *)&data[iterator->key_offset];
}

static int
xattr_value_indirect_load(struct HsqsXattrTableIterator *iterator) {
	const struct HsqsXattrValue *value = get_value(iterator);
	int rv = 0;
	if (hsqs_data_xattr_value_size(value) != 8) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	uint64_t ref = hsqs_data_xattr_value_ref(value);

	const struct HsqsXattrIdTable *header = get_header(iterator->context);
	uint64_t start_block = hsqs_data_xattr_id_table_xattr_table_start(header);
	rv = hsqs_metablock_stream_init(
			&iterator->out_of_line_value, iterator->context->hsqs, start_block,
			~0);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_metablock_stream_seek_ref(&iterator->out_of_line_value, ref);
	if (rv < 0) {
		goto out;
	}
	size_t size = HSQS_SIZEOF_XATTR_VALUE;
	rv = hsqs_metablock_stream_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}

	// Value offset 0 marks an indirect load.
	iterator->value_offset = 0;
	value = get_value(iterator);
	size += hsqs_data_xattr_value_size(value);
	rv = hsqs_metablock_stream_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
hsqs_xattr_table_iterator_next(struct HsqsXattrTableIterator *iterator) {
	int rv = 0;
	off_t offset = iterator->next_offset;
	size_t size = offset;

	hsqs_metablock_stream_cleanup(&iterator->out_of_line_value);

	if (iterator->remaining_entries == 0) {
		return 0;
	}

	// Load Key Header
	size += HSQS_SIZEOF_XATTR_KEY;
	rv = hsqs_metablock_stream_more(&iterator->metablock, size);
	if (rv < 0) {
		goto out;
	}
	iterator->key_offset = offset;

	// Load Key Name
	size += hsqs_xattr_table_iterator_name_size(iterator);
	rv = hsqs_metablock_stream_more(&iterator->metablock, size);
	if (rv < 0) {
		goto out;
	}

	// Load Value Header
	offset = size;
	size += HSQS_SIZEOF_XATTR_VALUE;
	rv = hsqs_metablock_stream_more(&iterator->metablock, size);
	if (rv < 0) {
		goto out;
	}
	iterator->value_offset = offset;

	// Load Value
	size += hsqs_xattr_table_iterator_value_size(iterator);
	rv = hsqs_metablock_stream_more(&iterator->metablock, size);
	if (rv < 0) {
		goto out;
	}

	iterator->next_offset = size;

	if (hsqs_xattr_table_iterator_is_indirect(iterator)) {
		rv = xattr_value_indirect_load(iterator);
		if (rv < 0) {
			goto out;
		}
	}

	rv = iterator->remaining_entries;

	iterator->remaining_entries--;

out:
	return rv;
}

uint16_t
hsqs_xattr_table_iterator_type(struct HsqsXattrTableIterator *iterator) {
	return hsqs_data_xattr_key_type(get_key(iterator)) & ~0x0100;
}

bool
hsqs_xattr_table_iterator_is_indirect(struct HsqsXattrTableIterator *iterator) {
	return (hsqs_data_xattr_key_type(get_key(iterator)) & 0x0100) != 0;
}

const char *
hsqs_xattr_table_iterator_name(struct HsqsXattrTableIterator *iterator) {
	return (const char *)hsqs_data_xattr_key_name(get_key(iterator));
}

const char *
hsqs_xattr_table_iterator_prefix(struct HsqsXattrTableIterator *iterator) {
	switch (hsqs_xattr_table_iterator_type(iterator)) {
	case HSQS_XATTR_USER:
		return "user.";
	case HSQS_XATTR_TRUSTED:
		return "trusted.";
	case HSQS_XATTR_SECURITY:
		return "security.";
	}
	return NULL;
}

uint16_t
hsqs_xattr_table_iterator_prefix_size(struct HsqsXattrTableIterator *iterator) {
	return strlen(hsqs_xattr_table_iterator_prefix(iterator));
}

int
hsqs_xattr_table_iterator_fullname_dup(
		struct HsqsXattrTableIterator *iterator, char **fullname_buffer) {
	const char *prefix = hsqs_xattr_table_iterator_prefix(iterator);
	size_t name_size = hsqs_xattr_table_iterator_name_size(iterator);
	size_t size = strlen(prefix) + name_size;

	*fullname_buffer = calloc(size + 1, sizeof(char));
	if (*fullname_buffer) {
		strcpy(*fullname_buffer, prefix);
		strncat(*fullname_buffer, hsqs_xattr_table_iterator_name(iterator),
				name_size);
		return size;
	} else {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
}

uint16_t
hsqs_xattr_table_iterator_name_size(struct HsqsXattrTableIterator *iterator) {
	return hsqs_data_xattr_key_name_size(get_key(iterator));
}

int
hsqs_xattr_table_iterator_value_dup(
		struct HsqsXattrTableIterator *iterator, char **value_buffer) {
	int size = hsqs_xattr_table_iterator_value_size(iterator);
	const char *value = hsqs_xattr_table_iterator_value(iterator);

	*value_buffer = hsqs_memdup(value, size);
	if (*value_buffer) {
		return size;
	} else {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
}

const char *
hsqs_xattr_table_iterator_value(struct HsqsXattrTableIterator *iterator) {
	return (const char *)hsqs_data_xattr_value(get_value(iterator));
}

uint16_t
hsqs_xattr_table_iterator_value_size(struct HsqsXattrTableIterator *iterator) {
	return hsqs_data_xattr_value_size(get_value(iterator));
}

int
hsqs_xattr_table_iterator_cleanup(struct HsqsXattrTableIterator *iterator) {
	hsqs_metablock_stream_cleanup(&iterator->out_of_line_value);
	hsqs_metablock_stream_cleanup(&iterator->metablock);
	return 0;
}
