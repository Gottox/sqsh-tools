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
 * @created     : Sunday Dec 19, 2021 22:13:26 CET
 */

#include "../context/metablock_stream_context.h"
#include "../utils.h"

#ifndef XATTR_CONTEXT_H

struct HsqsInodeContext;

struct HsqsXattrIterator {
	struct HsqsMetablockStreamContext metablock;
	struct HsqsMetablockStreamContext out_of_line_value;
	struct HsqsXattrTable *context;
	int remaining_entries;
	hsqs_index_t next_offset;
	hsqs_index_t key_offset;
	hsqs_index_t value_offset;
};

HSQS_NO_UNUSED int hsqs_xattr_iterator_init(
		struct HsqsXattrIterator *iterator, struct HsqsXattrTable *xattr_table,
		const struct HsqsInodeContext *inode);

int hsqs_xattr_iterator_next(struct HsqsXattrIterator *iterator);

uint16_t hsqs_xattr_iterator_type(struct HsqsXattrIterator *iterator);

bool hsqs_xattr_iterator_is_indirect(struct HsqsXattrIterator *iterator);

const char *hsqs_xattr_iterator_prefix(struct HsqsXattrIterator *iterator);
uint16_t hsqs_xattr_iterator_prefix_size(struct HsqsXattrIterator *iterator);
const char *hsqs_xattr_iterator_name(struct HsqsXattrIterator *iterator);
uint16_t hsqs_xattr_iterator_name_size(struct HsqsXattrIterator *iterator);
int hsqs_xattr_iterator_fullname_cmp(
		struct HsqsXattrIterator *iterator, const char *name);
int hsqs_xattr_iterator_fullname_dup(
		struct HsqsXattrIterator *iterator, char **fullname_buffer);

int hsqs_xattr_iterator_value_dup(
		struct HsqsXattrIterator *iterator, char **value_buffer);

const char *hsqs_xattr_iterator_value(struct HsqsXattrIterator *iterator);

uint16_t hsqs_xattr_iterator_value_size(struct HsqsXattrIterator *iterator);

int hsqs_xattr_iterator_cleanup(struct HsqsXattrIterator *iterator);

#define XATTR_CONTEXT_H

#endif /* end of include guard XATTR_CONTEXT_H */
