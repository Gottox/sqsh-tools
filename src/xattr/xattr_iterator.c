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

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         xattr_iterator.c
 */

#include "../../include/sqsh_xattr_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_inode_private.h"

int
sqsh__xattr_iterator_init(
		struct SqshXattrIterator *iterator,
		const struct SqshInodeContext *inode) {
	int rv;
	struct SqshDataXattrLookupTable ref = {0};
	struct SqshXattrTable *xattr_table = NULL;
	struct SqshArchive *sqsh = inode->sqsh;
	const struct SqshSuperblockContext *superblock =
			sqsh_archive_superblock(sqsh);
	uint32_t index = sqsh_inode_xattr_index(inode);

	rv = sqsh_archive_xattr_table(sqsh, &xattr_table);
	if (rv < 0) {
		return rv;
	}

	if (index == SQSH_INODE_NO_XATTR) {
		iterator->remaining_entries = 0;
		return 0;
	}
	if (index != SQSH_INODE_NO_XATTR && xattr_table == NULL) {
		// TODO: Be more specific about the error code. This incidates
		// not only a missing xattr table but also links into the non
		// existing table. So the archive must be considerred corrupt.
		return -SQSH_ERROR_NO_XATTR_TABLE;
	}

	rv = sqsh_xattr_table_get(xattr_table, index, &ref);
	if (rv < 0) {
		goto out;
	}

	// The XATTR table is the last block in the file system.
	const uint64_t address_ref = sqsh_data_xattr_lookup_table_xattr_ref(&ref);
	const uint64_t inner_offset = sqsh_address_ref_inner_offset(address_ref);
	const uint64_t outer_offset = sqsh_address_ref_outer_offset(address_ref);
	const uint64_t archive_size = sqsh_superblock_bytes_used(superblock);

	uint64_t start_block = sqsh_xattr_table_start(xattr_table);
	if (SQSH_ADD_OVERFLOW(start_block, outer_offset, &start_block)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = sqsh__metablock_reader_init(
			&iterator->metablock, sqsh, start_block, archive_size);
	if (rv < 0) {
		goto out;
	}

	iterator->remaining_entries = sqsh_data_xattr_lookup_table_count(&ref);
	iterator->next_offset = inner_offset;
	iterator->value_index = 0;
	iterator->context = xattr_table;
	iterator->sqsh = sqsh;
	iterator->upper_limit = archive_size;

out:
	if (rv < 0) {
		sqsh__xattr_iterator_cleanup(iterator);
	}
	return rv;
}

struct SqshXattrIterator *
sqsh_xattr_iterator_new(const struct SqshInodeContext *inode, int *err) {
	struct SqshXattrIterator *iterator =
			calloc(1, sizeof(struct SqshXattrIterator));
	if (iterator == NULL) {
		return NULL;
	}
	*err = sqsh__xattr_iterator_init(iterator, inode);
	if (*err < 0) {
		free(iterator);
		return NULL;
	}
	return iterator;
}

static const struct SqshDataXattrValue *
get_value(struct SqshXattrIterator *iterator) {
	const struct SqshMetablockReader *source;

	if (iterator->value_index == 0) {
		source = &iterator->out_of_line_value;
	} else {
		source = &iterator->metablock;
	}

	const uint8_t *data = sqsh__metablock_reader_data(source);
	return (const struct SqshDataXattrValue *)&data[iterator->value_index];
}

static const struct SqshDataXattrKey *
get_key(struct SqshXattrIterator *iterator) {
	const uint8_t *data = sqsh__metablock_reader_data(&iterator->metablock);

	return (const struct SqshDataXattrKey *)data;
}

static int
xattr_value_indirect_load(struct SqshXattrIterator *iterator) {
	const struct SqshDataXattrValue *value = get_value(iterator);
	int rv = 0;
	if (sqsh_data_xattr_value_size(value) != 8) {
		return -SQSH_ERROR_SIZE_MISSMATCH;
	}
	uint64_t ref = sqsh_data_xattr_value_ref(value);
	uint64_t outer_offset = sqsh_address_ref_outer_offset(ref);
	uint16_t inner_offset = sqsh_address_ref_inner_offset(ref);

	uint64_t start_block = sqsh_xattr_table_start(iterator->context);
	size_t size = SQSH_SIZEOF_XATTR_VALUE;
	if (SQSH_ADD_OVERFLOW(start_block, outer_offset, &start_block)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}
	rv = sqsh__metablock_reader_init(
			&iterator->out_of_line_value, iterator->sqsh, start_block,
			iterator->upper_limit);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__metablock_reader_advance(
			&iterator->out_of_line_value, inner_offset, size);
	if (rv < 0) {
		goto out;
	}

	// Value offset 0 marks an indirect load.
	iterator->value_index = 0;
	value = get_value(iterator);
	size += sqsh_data_xattr_value_size(value);
	rv = sqsh__metablock_reader_advance(&iterator->out_of_line_value, 0, size);
	if (rv < 0) {
		goto out;
	}

out:
	return rv;
}

int
sqsh_xattr_iterator_next(struct SqshXattrIterator *iterator) {
	int rv = 0;
	size_t size = SQSH_SIZEOF_XATTR_KEY + SQSH_SIZEOF_XATTR_VALUE;

	sqsh__metablock_reader_cleanup(&iterator->out_of_line_value);

	if (iterator->remaining_entries == 0) {
		return 0;
	}

	// Load Key Header
	rv = sqsh__metablock_reader_advance(
			&iterator->metablock, iterator->next_offset, SQSH_SIZEOF_XATTR_KEY);
	if (rv < 0) {
		goto out;
	}

	// Load Key Name
	const uint16_t name_size = sqsh_xattr_iterator_name_size(iterator);
	size += name_size;
	rv = sqsh__metablock_reader_advance(&iterator->metablock, 0, size);
	if (rv < 0) {
		goto out;
	}

	// Load Value Header
	iterator->value_index = SQSH_SIZEOF_XATTR_KEY + name_size;

	// Load Value
	const uint16_t value_size = sqsh_xattr_iterator_value_size(iterator);
	size += value_size;
	rv = sqsh__metablock_reader_advance(&iterator->metablock, 0, size);
	if (rv < 0) {
		goto out;
	}

	iterator->next_offset = size;

	if (sqsh_xattr_iterator_is_indirect(iterator)) {
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
sqsh_xattr_iterator_type(struct SqshXattrIterator *iterator) {
	return sqsh_data_xattr_key_type(get_key(iterator)) & ~0x0100;
}

bool
sqsh_xattr_iterator_is_indirect(struct SqshXattrIterator *iterator) {
	return (sqsh_data_xattr_key_type(get_key(iterator)) & 0x0100) != 0;
}

const char *
sqsh_xattr_iterator_name(struct SqshXattrIterator *iterator) {
	return (const char *)sqsh_data_xattr_key_name(get_key(iterator));
}

const char *
sqsh_xattr_iterator_prefix(struct SqshXattrIterator *iterator) {
	switch (sqsh_xattr_iterator_type(iterator)) {
	case SQSH_XATTR_USER:
		return "user.";
	case SQSH_XATTR_TRUSTED:
		return "trusted.";
	case SQSH_XATTR_SECURITY:
		return "security.";
	}
	return NULL;
}

uint16_t
sqsh_xattr_iterator_prefix_size(struct SqshXattrIterator *iterator) {
	return strlen(sqsh_xattr_iterator_prefix(iterator));
}

int
sqsh_xattr_iterator_fullname_cmp(
		struct SqshXattrIterator *iterator, const char *name) {
	int rv = 0;
	const char *prefix = sqsh_xattr_iterator_prefix(iterator);
	int prefix_len = strlen(prefix);
	rv = strncmp(name, prefix, prefix_len);
	if (rv != 0) {
		return rv;
	}
	name += prefix_len;

	const char *xattr_name = sqsh_xattr_iterator_name(iterator);
	if (strlen(name) != sqsh_xattr_iterator_name_size(iterator)) {
		return -1;
	} else {
		return strncmp(
				name, xattr_name, sqsh_xattr_iterator_name_size(iterator));
	}
}

int
sqsh_xattr_iterator_fullname_dup(
		struct SqshXattrIterator *iterator, char **fullname_buffer) {
	const char *prefix = sqsh_xattr_iterator_prefix(iterator);
	size_t name_size = sqsh_xattr_iterator_name_size(iterator);
	size_t size = strlen(prefix) + name_size;

	*fullname_buffer = calloc(size + 1, sizeof(char));
	if (*fullname_buffer) {
		strcpy(*fullname_buffer, prefix);
		strncat(*fullname_buffer, sqsh_xattr_iterator_name(iterator),
				name_size);
		return size;
	} else {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
}

uint16_t
sqsh_xattr_iterator_name_size(struct SqshXattrIterator *iterator) {
	return sqsh_data_xattr_key_name_size(get_key(iterator));
}

int
sqsh_xattr_iterator_value_dup(
		struct SqshXattrIterator *iterator, char **value_buffer) {
	int size = sqsh_xattr_iterator_value_size(iterator);
	const char *value = sqsh_xattr_iterator_value(iterator);

	*value_buffer = sqsh_memdup(value, size);
	if (*value_buffer) {
		return size;
	} else {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
}

const char *
sqsh_xattr_iterator_value(struct SqshXattrIterator *iterator) {
	return (const char *)sqsh_data_xattr_value(get_value(iterator));
}

uint16_t
sqsh_xattr_iterator_value_size(struct SqshXattrIterator *iterator) {
	return sqsh_data_xattr_value_size(get_value(iterator));
}

int
sqsh__xattr_iterator_cleanup(struct SqshXattrIterator *iterator) {
	sqsh__metablock_reader_cleanup(&iterator->out_of_line_value);
	sqsh__metablock_reader_cleanup(&iterator->metablock);
	return 0;
}

int
sqsh_xattr_iterator_free(struct SqshXattrIterator *iterator) {
	if (iterator == NULL) {
		return 0;
	}
	int rv = sqsh__xattr_iterator_cleanup(iterator);
	free(iterator);
	return rv;
}
