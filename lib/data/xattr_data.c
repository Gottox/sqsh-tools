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
 * @file         xattr_data.c
 */

#define _DEFAULT_SOURCE

#include "../../include/sqsh_data_private.h"

#if defined(__FreeBSD__)
#	include <sys/endian.h>
#else
#	include <endian.h>
#endif

#include <string.h>

struct SQSH_UNALIGNED SqshDataXattrKey {
	uint16_t type;
	uint16_t name_size;
	/* uint8_t name[0]; // [name_size - strlen(prefix)]; */
};
SQSH_STATIC_ASSERT(sizeof(struct SqshDataXattrKey) == SQSH_SIZEOF_XATTR_KEY);

struct SQSH_UNALIGNED SqshDataXattrValue {
	uint32_t value_size;
	/* uint8_t value[0]; // [value_size] */
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrValue) == SQSH_SIZEOF_XATTR_VALUE);

struct SQSH_UNALIGNED SqshDataXattrLookupTable {
	uint64_t xattr_ref;
	uint32_t count;
	uint32_t size;
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrLookupTable) ==
		SQSH_SIZEOF_XATTR_LOOKUP_TABLE);

struct SQSH_UNALIGNED SqshDataXattrIdTable {
	uint64_t xattr_table_start;
	uint32_t xattr_ids;
	uint32_t _unused;
	/* uint64_t table[0]; // [ceil(xattr_ids / 512.0)] */
};
SQSH_STATIC_ASSERT(
		sizeof(struct SqshDataXattrIdTable) == SQSH_SIZEOF_XATTR_ID_TABLE);

uint16_t
sqsh__data_xattr_key_type(const struct SqshDataXattrKey *xattr_key) {
	return le16toh(xattr_key->type);
}

uint16_t
sqsh__data_xattr_key_name_size(const struct SqshDataXattrKey *xattr_key) {
	return le16toh(xattr_key->name_size);
}
const uint8_t *
sqsh__data_xattr_key_name(const struct SqshDataXattrKey *xattr_key) {
	return (const uint8_t *)&xattr_key[1];
}

uint32_t
sqsh__data_xattr_value_size(const struct SqshDataXattrValue *xattr_value) {
	return le32toh(xattr_value->value_size);
}
const uint8_t *
sqsh__data_xattr_value(const struct SqshDataXattrValue *xattr_value) {
	return (const uint8_t *)&xattr_value[1];
}
uint64_t
sqsh__data_xattr_value_ref(const struct SqshDataXattrValue *xattr_value) {
	uint64_t ref = 0;

	memcpy(&ref, sqsh__data_xattr_value(xattr_value), sizeof(uint64_t));
	return le64toh(ref);
}

uint64_t
sqsh__data_xattr_lookup_table_xattr_ref(
		const struct SqshDataXattrLookupTable *lookup_table) {
	return le64toh(lookup_table->xattr_ref);
}
uint32_t
sqsh__data_xattr_lookup_table_count(
		const struct SqshDataXattrLookupTable *lookup_table) {
	return le32toh(lookup_table->count);
}
uint32_t
sqsh__data_xattr_lookup_table_size(
		const struct SqshDataXattrLookupTable *lookup_table) {
	return le32toh(lookup_table->size);
}

uint64_t
sqsh__data_xattr_id_table_xattr_table_start(
		const struct SqshDataXattrIdTable *xattr_id_table) {
	return le64toh(xattr_id_table->xattr_table_start);
}
uint32_t
sqsh__data_xattr_id_table_xattr_ids(
		const struct SqshDataXattrIdTable *xattr_id_table) {
	return le32toh(xattr_id_table->xattr_ids);
}
