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
 * @created     : Sunday Oct 31, 2021 11:54:43 CET
 */

#include "../utils.h"
#include "metablock_context.h"
#include "metablock_stream_context.h"
#include "table_context.h"
#include <stdint.h>

#ifndef XATTR_TABLE_CONTEXT_H

#define XATTR_TABLE_CONTEXT_H

struct HsqsSuperblockContext;
struct HsqsXattrKey;
struct HsqsXattrValue;
struct HsqsInodeContext;

struct HsqsXattrTableContext {
	struct Hsqs *hsqs;
	struct HsqsMap header;
	struct HsqsTableContext table;
};

struct HsqsXattrTableIterator {
	struct HsqsMetablockStreamContext metablock;
	struct HsqsMetablockStreamContext out_of_line_value;
	struct HsqsXattrTableContext *context;
	int remaining_entries;
	off_t next_offset;
	off_t key_offset;
	off_t value_offset;
};

HSQS_NO_UNUSED int
hsqs_xattr_table_init(struct HsqsXattrTableContext *context, struct Hsqs *hsqs);

HSQS_NO_UNUSED int hsqs_xattr_table_iterator_init(
		struct HsqsXattrTableIterator *iterator,
		struct HsqsXattrTableContext *xattr_table,
		const struct HsqsInodeContext *inode);

int hsqs_xattr_table_iterator_next(struct HsqsXattrTableIterator *iterator);

uint16_t
hsqs_xattr_table_iterator_type(struct HsqsXattrTableIterator *iterator);

bool
hsqs_xattr_table_iterator_is_indirect(struct HsqsXattrTableIterator *iterator);

const char *
hsqs_xattr_table_iterator_prefix(struct HsqsXattrTableIterator *iterator);
uint16_t
hsqs_xattr_table_iterator_prefix_size(struct HsqsXattrTableIterator *iterator);
const char *
hsqs_xattr_table_iterator_name(struct HsqsXattrTableIterator *iterator);
uint16_t
hsqs_xattr_table_iterator_name_size(struct HsqsXattrTableIterator *iterator);
int hsqs_xattr_table_iterator_fullname_dup(
		struct HsqsXattrTableIterator *iterator, char **fullname_buffer);

int hsqs_xattr_table_iterator_value_dup(
		struct HsqsXattrTableIterator *iterator, char **value_buffer);

const char *
hsqs_xattr_table_iterator_value(struct HsqsXattrTableIterator *iterator);

uint16_t
hsqs_xattr_table_iterator_value_size(struct HsqsXattrTableIterator *iterator);

int hsqs_xattr_table_iterator_cleanup(struct HsqsXattrTableIterator *iterator);

int hsqs_xattr_table_cleanup(struct HsqsXattrTableContext *context);

#endif /* end of include guard XATTR_TABLE_CONTEXT_H */
