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
 * @file         xattr.h
 */

#include <stdint.h>

#ifndef HSQS_XATTR_H

#define HSQS_XATTR_H

#define HSQS_SIZEOF_XATTR_KEY 4
#define HSQS_SIZEOF_XATTR_VALUE 4
#define HSQS_SIZEOF_XATTR_LOOKUP_TABLE 16
#define HSQS_SIZEOF_XATTR_ID_TABLE 16

enum SqshXattrType {
	HSQS_XATTR_USER = 0,
	HSQS_XATTR_TRUSTED = 1,
	HSQS_XATTR_SECURITY = 2,
};

struct SqshXattrKey;

struct SqshXattrValue;

struct SqshXattrLookupTable;

struct SqshXattrIdTable;

uint16_t sqsh_data_xattr_key_type(const struct SqshXattrKey *xattr_key);
uint16_t sqsh_data_xattr_key_name_size(const struct SqshXattrKey *xattr_key);
const uint8_t *sqsh_data_xattr_key_name(const struct SqshXattrKey *xattr_key);

uint32_t sqsh_data_xattr_value_size(const struct SqshXattrValue *xattr_value);
uint64_t sqsh_data_xattr_value_ref(const struct SqshXattrValue *xattr_value);
const uint8_t *sqsh_data_xattr_value(const struct SqshXattrValue *xattr_value);

uint64_t sqsh_data_xattr_lookup_table_xattr_ref(
		const struct SqshXattrLookupTable *lookup_table);
uint32_t sqsh_data_xattr_lookup_table_count(
		const struct SqshXattrLookupTable *lookup_table);
uint32_t sqsh_data_xattr_lookup_table_size(
		const struct SqshXattrLookupTable *lookup_table);

uint64_t sqsh_data_xattr_id_table_xattr_table_start(
		const struct SqshXattrIdTable *xattr_id_table);
uint32_t sqsh_data_xattr_id_table_xattr_ids(
		const struct SqshXattrIdTable *xattr_id_table);
const uint64_t *
sqsh_data_xattr_id_table(const struct SqshXattrIdTable *xattr_id_table);

#endif /* end of include guard HSQS_XATTR_H */
