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
get_xattr_prefix(enum SquashXattrType type) {
	switch (type) {
	case SQUASH_XATTR_USER:
		return "user.";
	case SQUASH_XATTR_TRUSTED:
		return "trusted.";
	case SQUASH_XATTR_SECURITY:
		return "security.";
	}
	return NULL;
}

int
squash_xattr_table_init(
		struct SquashXattrTableContext *context,
		struct SquashSuperblockContext *superblock) {
	int rv = 0;
	uint64_t offset =
			squash_data_superblock_xattr_id_table_start(superblock->superblock);
	uint64_t bytes_used = squash_superblock_bytes_used(superblock);
	if (offset + SQUASH_SIZEOF_XATTR_ID_TABLE >= bytes_used) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}
	context->header = squash_superblock_data_from_offset(superblock, offset);
	context->superblock = superblock;
	if (context->header == NULL) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}

	rv = squash_table_init(
			&context->table, superblock, offset + SQUASH_SIZEOF_XATTR_ID_TABLE,
			SQUASH_SIZEOF_XATTR_LOOKUP_TABLE,
			squash_data_xattr_id_table_xattr_ids(context->header));
	return rv;
}

int
squash_xattr_table_iterator_init(
		struct SquashXattrTableIterator *iterator,
		struct SquashXattrTableContext *xattr_table,
		const struct SquashInodeContext *inode) {
	int rv;
	const struct SquashXattrLookupTable *ref;
	uint32_t index = squash_inode_xattr_index(inode);

	if (index == SQUASH_INODE_NO_XATTR) {
		iterator->remaining_entries = 0;
		return 0;
	}

	rv = squash_table_get(&xattr_table->table, index, (const void **)&ref);
	if (rv < 0) {
		squash_xattr_table_iterator_cleanup(iterator);
		goto out;
	}

	// TODO: references are used in inodes too. Generalize!
	uint64_t start_block =
			squash_data_xattr_id_table_xattr_table_start(xattr_table->header);
	rv = squash_metablock_init(
			&iterator->metablock, xattr_table->superblock, start_block);
	if (rv < 0) {
		squash_xattr_table_iterator_cleanup(iterator);
		goto out;
	}
	uint64_t metablock_index =
			squash_data_xattr_lookup_table_xattr_ref(ref) >> 16;
	uint16_t metablock_offset =
			squash_data_xattr_lookup_table_xattr_ref(ref) & 0xFFFF;
	rv = squash_metablock_seek(
			&iterator->metablock, metablock_index, metablock_offset);
	if (rv < 0) {
		squash_xattr_table_iterator_cleanup(iterator);
		goto out;
	}

	iterator->remaining_entries = squash_data_xattr_lookup_table_count(ref);

	rv = squash_metablock_more(
			&iterator->metablock, squash_data_xattr_lookup_table_size(ref));

	iterator->current_key = NULL;
	iterator->current_value = NULL;
	iterator->context = xattr_table;

out:
	return rv;
}

int
xattr_value_indirect_load(
		struct SquashXattrTableIterator *iterator,
		struct SquashXattrValue *ref_value) {
	struct SquashXattrValue *tmp;
	int rv = 0;
	if (squash_data_xattr_value_size(ref_value) != 8) {
		return -SQUASH_ERROR_SIZE_MISSMATCH;
	}
	uint64_t ref = squash_data_xattr_value_ref(ref_value);

	uint64_t start_block = squash_data_xattr_id_table_xattr_table_start(
			iterator->context->header);
	rv = squash_metablock_init(
			&iterator->out_of_line_value, iterator->context->superblock,
			start_block);
	if (rv < 0) {
		goto out;
	}
	uint64_t metablock_index = ref >> 16;
	uint16_t metablock_offset = ref & 0xFFFF;
	rv = squash_metablock_seek(
			&iterator->out_of_line_value, metablock_index, metablock_offset);
	if (rv < 0) {
		goto out;
	}
	size_t size = SQUASH_SIZEOF_XATTR_VALUE;
	rv = squash_metablock_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}
	tmp = (struct SquashXattrValue *)squash_metablock_data(
			&iterator->out_of_line_value);
	size += squash_data_xattr_value_size(tmp);
	rv = squash_metablock_more(&iterator->out_of_line_value, size);
	if (rv < 0) {
		goto out;
	}
	iterator->current_value = (struct SquashXattrValue *)squash_metablock_data(
			&iterator->out_of_line_value);

out:
	return rv;
}

int
squash_xattr_table_iterator_next(struct SquashXattrTableIterator *iterator) {
	int rv = 0;
	struct SquashXattrValue *value;
	int remaining_entries = iterator->remaining_entries;
	if (iterator->remaining_entries == 0) {
		return 0;
	}
	if (iterator->current_key == NULL) {
		iterator->current_key = (struct SquashXattrKey *)squash_metablock_data(
				&iterator->metablock);
	} else {
		if (squash_xattr_table_iterator_is_indirect(iterator)) {
			squash_metablock_cleanup(&iterator->out_of_line_value);
		}
		iterator->current_key =
				(struct SquashXattrKey *)&squash_data_xattr_value(
						iterator->current_value)[squash_data_xattr_value_size(
						iterator->current_value)];
	}

	value = (struct SquashXattrValue *)&squash_data_xattr_key_name(
			iterator->current_key)[squash_data_xattr_key_name_size(
			iterator->current_key)];

	if (squash_xattr_table_iterator_is_indirect(iterator)) {
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
squash_xattr_table_iterator_type(struct SquashXattrTableIterator *iterator) {
	return squash_data_xattr_key_type(iterator->current_key) & ~0x0100;
}

bool
squash_xattr_table_iterator_is_indirect(
		struct SquashXattrTableIterator *iterator) {
	return (squash_data_xattr_key_type(iterator->current_key) & 0x0100) != 0;
}

const char *
squash_xattr_table_iterator_name(struct SquashXattrTableIterator *iterator) {
	return (const char *)squash_data_xattr_key_name(iterator->current_key);
}

int
squash_xattr_table_iterator_fullname_dup(
		struct SquashXattrTableIterator *iterator, char **fullname_buffer) {
	const char *prefix =
			get_xattr_prefix(squash_xattr_table_iterator_type(iterator));
	size_t name_size = squash_xattr_table_iterator_name_size(iterator);
	size_t size = strlen(prefix) + name_size;

	*fullname_buffer = calloc(size + 1, sizeof(char));
	if (*fullname_buffer) {
		strcpy(*fullname_buffer, prefix);
		strncat(*fullname_buffer, squash_xattr_table_iterator_name(iterator),
				name_size);
		return size;
	} else {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
}

uint16_t
squash_xattr_table_iterator_name_size(
		struct SquashXattrTableIterator *iterator) {
	return squash_data_xattr_key_name_size(iterator->current_key);
}

int
squash_xattr_table_iterator_value_dup(
		struct SquashXattrTableIterator *iterator, char **value_buffer) {
	int size = squash_xattr_table_iterator_value_size(iterator);
	const char *value = squash_xattr_table_iterator_value(iterator);

	*value_buffer = squash_memdup(value, size);
	if (*value_buffer) {
		return size;
	} else {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}
}

const char *
squash_xattr_table_iterator_value(struct SquashXattrTableIterator *iterator) {
	return (const char *)squash_data_xattr_value(iterator->current_value);
}

uint16_t
squash_xattr_table_iterator_value_size(
		struct SquashXattrTableIterator *iterator) {
	return squash_data_xattr_value_size(iterator->current_value);
}

int
squash_xattr_table_iterator_cleanup(struct SquashXattrTableIterator *iterator) {
	squash_metablock_cleanup(&iterator->out_of_line_value);
	squash_metablock_cleanup(&iterator->metablock);
	return 0;
}

int
squash_xattr_table_cleanup(struct SquashXattrTableContext *context) {
	squash_table_cleanup(&context->table);
	return 0;
}
