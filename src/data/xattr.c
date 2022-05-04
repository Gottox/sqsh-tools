/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         xattr.c
 */

#include "xattr_internal.h"
#include <endian.h>
#include <stdint.h>
#include <string.h>

uint16_t
hsqs_data_xattr_key_type(const struct HsqsXattrKey *xattr_key) {
	return le16toh(xattr_key->type);
}

uint16_t
hsqs_data_xattr_key_name_size(const struct HsqsXattrKey *xattr_key) {
	return le16toh(xattr_key->name_size);
}
const uint8_t *
hsqs_data_xattr_key_name(const struct HsqsXattrKey *xattr_key) {
	return (const uint8_t *)&xattr_key[1];
}

uint32_t
hsqs_data_xattr_value_size(const struct HsqsXattrValue *xattr_value) {
	return le32toh(xattr_value->value_size);
}
const uint8_t *
hsqs_data_xattr_value(const struct HsqsXattrValue *xattr_value) {
	return (const uint8_t *)&xattr_value[1];
}
uint64_t
hsqs_data_xattr_value_ref(const struct HsqsXattrValue *xattr_value) {
	uint64_t ref = 0;

	memcpy(&ref, hsqs_data_xattr_value(xattr_value), sizeof(uint64_t));
	return le64toh(ref);
}

uint64_t
hsqs_data_xattr_lookup_table_xattr_ref(
		const struct HsqsXattrLookupTable *lookup_table) {
	return le64toh(lookup_table->xattr_ref);
}
uint32_t
hsqs_data_xattr_lookup_table_count(
		const struct HsqsXattrLookupTable *lookup_table) {
	return le32toh(lookup_table->count);
}
uint32_t
hsqs_data_xattr_lookup_table_size(
		const struct HsqsXattrLookupTable *lookup_table) {
	return le32toh(lookup_table->size);
}

uint64_t
hsqs_data_xattr_id_table_xattr_table_start(
		const struct HsqsXattrIdTable *xattr_id_table) {
	return le64toh(xattr_id_table->xattr_table_start);
}
uint32_t
hsqs_data_xattr_id_table_xattr_ids(
		const struct HsqsXattrIdTable *xattr_id_table) {
	return le32toh(xattr_id_table->xattr_ids);
}
uint64_t
hsqs_data_xattr_id_table_ref(
		const struct HsqsXattrIdTable *xattr_id_table, uint64_t index) {
	const uint64_t *table = (const uint64_t *)&xattr_id_table[1];

	return le64toh(table[index]);
}
