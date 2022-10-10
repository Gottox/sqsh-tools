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
 * @file         xattr_iterator.h
 */

#include "../context/metablock_stream_context.h"
#include "../utils.h"

#ifndef XATTR_CONTEXT_H

struct SqshInodeContext;

struct SqshXattrIterator {
	struct SqshMetablockStreamContext metablock;
	struct SqshMetablockStreamContext out_of_line_value;
	struct SqshXattrTable *context;
	int remaining_entries;
	sqsh_index_t next_offset;
	sqsh_index_t key_offset;
	sqsh_index_t value_offset;
};

SQSH_NO_UNUSED int sqsh_xattr_iterator_init(
		struct SqshXattrIterator *iterator, struct SqshXattrTable *xattr_table,
		const struct SqshInodeContext *inode);

int sqsh_xattr_iterator_next(struct SqshXattrIterator *iterator);

uint16_t sqsh_xattr_iterator_type(struct SqshXattrIterator *iterator);

bool sqsh_xattr_iterator_is_indirect(struct SqshXattrIterator *iterator);

const char *sqsh_xattr_iterator_prefix(struct SqshXattrIterator *iterator);
uint16_t sqsh_xattr_iterator_prefix_size(struct SqshXattrIterator *iterator);
const char *sqsh_xattr_iterator_name(struct SqshXattrIterator *iterator);
uint16_t sqsh_xattr_iterator_name_size(struct SqshXattrIterator *iterator);
int sqsh_xattr_iterator_fullname_cmp(
		struct SqshXattrIterator *iterator, const char *name);
int sqsh_xattr_iterator_fullname_dup(
		struct SqshXattrIterator *iterator, char **fullname_buffer);

int sqsh_xattr_iterator_value_dup(
		struct SqshXattrIterator *iterator, char **value_buffer);

const char *sqsh_xattr_iterator_value(struct SqshXattrIterator *iterator);

uint16_t sqsh_xattr_iterator_value_size(struct SqshXattrIterator *iterator);

int sqsh_xattr_iterator_cleanup(struct SqshXattrIterator *iterator);

#define XATTR_CONTEXT_H

#endif /* end of include guard XATTR_CONTEXT_H */
