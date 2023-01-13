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
 * @file         sqsh_iterator.h
 */

#ifndef SQSH_ITERATOR_H
#define SQSH_ITERATOR_H

#include "sqsh_common.h"
#include "sqsh_context.h"

#ifdef __cplusplus
extern "C" {
#endif

// iterator/directory_index_iterator.c

struct SqshInodeDirectoryIndexIterator {
	struct SqshInodeContext *inode;
	size_t remaining_entries;
	sqsh_index_t current_offset;
	sqsh_index_t next_offset;
};

SQSH_NO_UNUSED int sqsh_inode_directory_index_iterator_init(
		struct SqshInodeDirectoryIndexIterator *iterator,
		struct SqshInodeContext *inode);
SQSH_NO_UNUSED
struct SqshInodeDirectoryIndexIterator *sqsh_inode_directory_index_iterator_new(
		struct SqshInodeContext *inode, int *err);
SQSH_NO_UNUSED int sqsh_inode_directory_index_iterator_next(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_index(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_start(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_name_size(
		struct SqshInodeDirectoryIndexIterator *iterator);
const char *sqsh_inode_directory_index_iterator_name(
		struct SqshInodeDirectoryIndexIterator *iterator);
int sqsh_inode_directory_index_iterator_cleanup(
		struct SqshInodeDirectoryIndexIterator *iterator);
int sqsh_inode_directory_index_iterator_free(
		struct SqshInodeDirectoryIndexIterator *iterator);

// iterator/directory_iterator.c

struct SqshInodeContext;
struct Sqsh;

struct SqshDirectoryIterator {
	struct SqshInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;

	const struct SqshDirectoryFragment *fragments;
	struct SqshDirectoryContext *directory;
	struct SqshMetablockStreamContext metablock;
	size_t remaining_entries;
	sqsh_index_t current_fragment_offset;
	sqsh_index_t next_offset;
	sqsh_index_t current_offset;
};

SQSH_NO_UNUSED int sqsh_directory_iterator_init(
		struct SqshDirectoryIterator *iterator, struct SqshInodeContext *inode);
SQSH_NO_UNUSED struct SqshDirectoryIterator *
sqsh_directory_iterator_new(struct SqshInodeContext *inode, int *err);
SQSH_NO_UNUSED int
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator);
SQSH_NO_UNUSED int sqsh_directory_iterator_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len);
int
sqsh_directory_iterator_name_size(const struct SqshDirectoryIterator *iterator);
uint64_t
sqsh_directory_iterator_inode_ref(const struct SqshDirectoryIterator *iterator);
enum SqshInodeContextType sqsh_directory_iterator_inode_type(
		const struct SqshDirectoryIterator *iterator);
SQSH_NO_UNUSED int sqsh_directory_iterator_inode_load(
		const struct SqshDirectoryIterator *iterator,
		struct SqshInodeContext *inode);
const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator);
SQSH_NO_UNUSED int sqsh_directory_iterator_name_dup(
		const struct SqshDirectoryIterator *iterator, char **name_buffer);
int sqsh_directory_iterator_cleanup(struct SqshDirectoryIterator *iterator);
int sqsh_directory_iterator_free(struct SqshDirectoryIterator *iterator);

// iterator/xattr_iterator.c

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
		struct SqshXattrIterator *iterator,
		const struct SqshInodeContext *inode);
SQSH_NO_UNUSED struct SqshXattrIterator *
sqsh_xattr_iterator_new(const struct SqshInodeContext *inode, int *err);

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
int sqsh_xattr_iterator_free(struct SqshXattrIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_ITERATOR_H */
