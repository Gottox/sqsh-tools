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
 * @file        : xattr_table_context
 * @created     : Sunday Oct 31, 2021 11:54:09 CET
 */

#include "xattr_table_context.h"
#include "../compression/buffer.h"
#include "../data/superblock.h"
#include "../data/xattr.h"
#include "../error.h"
#include "inode_context.h"
#include "superblock_context.h"

#include <stdint.h>
#include <string.h>

static const char *
get_xattr_prefix(enum HsqsXattrType type) {
	switch (type) {
	case HSQS_XATTR_USER:
		return "user.";
	case HSQS_XATTR_TRUSTED:
		return "trusted.";
	case HSQS_XATTR_SECURITY:
		return "security.";
	}
	return NULL;
}

int
hsqs_xattr_table_init(
		struct HsqsXattrTableContext *context,
		struct HsqsSuperblockContext *superblock) {
	int rv = 0;
	uint64_t offset =
			hsqs_data_superblock_xattr_id_table_start(superblock->superblock);
	uint64_t bytes_used = hsqs_superblock_bytes_used(superblock);
	if (offset + HSQS_SIZEOF_XATTR_ID_TABLE >= bytes_used) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	context->header = hsqs_superblock_data_from_offset(superblock, offset);
	context->superblock = superblock;
	if (context->header == NULL) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}

	rv = hsqs_table_init(
			&context->table, superblock, offset + HSQS_SIZEOF_XATTR_ID_TABLE,
			HSQS_SIZEOF_XATTR_LOOKUP_TABLE,
			hsqs_data_xattr_id_table_xattr_ids(context->header));
	return rv;
}

int
hsqs_xattr_table_iterator_init(
		struct HsqsXattrTableIterator *iterator,
		struct HsqsXattrTableContext *xattr_table,
		const struct HsqsInodeContext *inode) {
	int rv;
	const struct HsqsXattrLookupTable *ref;
	uint32_t index = hsqs_inode_xattr_index(inode);

	if (index == HSQS_INODE_NO_XATTR) {
		iterator->remaining_entries = 0;
		return 0;
	}

	rv = hsqs_table_get(&xattr_table->table, index, (const void **)&ref);
	if (rv < 0) {
		hsqs_xattr_table_iterator_cleanup(iterator);
		goto out;
	}

	// TODO: references are used in inodes too. Generalize!
	uint64_t start_block =
			hsqs_data_xattr_id_table_xattr_table_start(xattr_table->header);
	rv = hsqs_metablock_init(
			&iterator->metablock, xattr_table->superblock, start_block);
	if (rv < 0) {
		hsqs_xattr_table_iterator_cleanup(iterator);
		goto out;
	}
	uint64_t metablock_index =
			hsqs_data_xattr_lookup_table_xattr_ref(ref) >> 16;
	uint16_t metablock_offset =
			hsqs_data_xattr_lookup_table_xattr_ref(ref) & 0xFFFF;
	rv = hsqs_metablock_seek(
			&iterator->metablock, metablock_index, metablock_offset);
	if (rv < 0) {
		hsqs_xattr_table_iterator_cleanup(iterator);
		goto out;
	}

	iterator->remaining_entries = hsqs_data_xattr_lookup_table_count(ref);

	rv = hsqs_metablock_more(
			&iterator->metablock, hsqs_data_xattr_lookup_table_size(ref));

	iterator->current_key = NULL;
	iterator->current_value = NULL;
	iterator->context = xattr_table;

out:
	return rv;
}

int
xattr_value_indirect_load(
		struct HsqsXattrTableIterator *iterator,
		struct HsqsXattrValue *ref_value) {
	struct HsqsXattrValue *tmp;
	int rv = 0;
	if (hsqs_data_xattr_value_size(ref_value) != 8) {
		return -HSQS_ERROR_SIZE_MISSMATCH;
	}
	uint64_t ref = hsqs_data_xattr_value_ref(ref_value);

	uint64_t start_block = hsqs_data_xattr_id_table_xattr_table_start(
			iterator->context->header);
	rv = hsqs_metablock_init(
			&iterator->out_of_line_value, iterator->context->superblock,
			start_block);
	if (rv < 0) {
		goto out;
	}
	uint64_t metablock_index = ref >> 16;
	uint16_t metablock_offset = ref & 0xFFFF;
	rv = hsqs_metablock_seek(
			&iterator->out_of_line_value, metablock_index, metablock_offset);
	if (rv < 0) {
		goto out;
	}
	size_t size = HSQS_SIZEOF_XATTR_VALUE;
	rv = hsqs_metablock_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}
	tmp = (struct HsqsXattrValue *)hsqs_metablock_data(
			&iterator->out_of_line_value);
	size += hsqs_data_xattr_value_size(tmp);
	rv = hsqs_metablock_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}
	iterator->current_value = (struct HsqsXattrValue *)hsqs_metablock_data(
			&iterator->out_of_line_value);

out:
	return rv;
}

int
hsqs_xattr_table_iterator_next(struct HsqsXattrTableIterator *iterator) {
	int rv = 0;
	struct HsqsXattrValue *value;
	int remaining_entries = iterator->remaining_entries;
	if (iterator->remaining_entries == 0) {
		return 0;
	}
	if (iterator->current_key == NULL) {
		iterator->current_key = (struct HsqsXattrKey *)hsqs_metablock_data(
				&iterator->metablock);
	} else {
		if (hsqs_xattr_table_iterator_is_indirect(iterator)) {
			hsqs_metablock_cleanup(&iterator->out_of_line_value);
		}
		iterator->current_key = (struct HsqsXattrKey *)&hsqs_data_xattr_value(
				iterator->current_value)[hsqs_data_xattr_value_size(
				iterator->current_value)];
	}

	value = (struct HsqsXattrValue *)&hsqs_data_xattr_key_name(
			iterator->current_key)[hsqs_data_xattr_key_name_size(
			iterator->current_key)];

	if (hsqs_xattr_table_iterator_is_indirect(iterator)) {
		rv = xattr_value_indirect_load(iterator, value);
		if (rv < 0) {
			return rv;
		}
	} else {
		iterator->current_value = value;
	}

	iterator->remaining_entries--;
	return remaining_entries;
}

uint16_t
hsqs_xattr_table_iterator_type(struct HsqsXattrTableIterator *iterator) {
	return hsqs_data_xattr_key_type(iterator->current_key) & ~0x0100;
}

bool
hsqs_xattr_table_iterator_is_indirect(struct HsqsXattrTableIterator *iterator) {
	return (hsqs_data_xattr_key_type(iterator->current_key) & 0x0100) != 0;
}

const char *
hsqs_xattr_table_iterator_name(struct HsqsXattrTableIterator *iterator) {
	return (const char *)hsqs_data_xattr_key_name(iterator->current_key);
}

int
hsqs_xattr_table_iterator_fullname_dup(
		struct HsqsXattrTableIterator *iterator, char **fullname_buffer) {
	const char *prefix =
			get_xattr_prefix(hsqs_xattr_table_iterator_type(iterator));
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
	return hsqs_data_xattr_key_name_size(iterator->current_key);
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
	return (const char *)hsqs_data_xattr_value(iterator->current_value);
}

uint16_t
hsqs_xattr_table_iterator_value_size(struct HsqsXattrTableIterator *iterator) {
	return hsqs_data_xattr_value_size(iterator->current_value);
}

int
hsqs_xattr_table_iterator_cleanup(struct HsqsXattrTableIterator *iterator) {
	hsqs_metablock_cleanup(&iterator->out_of_line_value);
	hsqs_metablock_cleanup(&iterator->metablock);
	return 0;
}

int
hsqs_xattr_table_cleanup(struct HsqsXattrTableContext *context) {
	hsqs_table_cleanup(&context->table);
	return 0;
}
