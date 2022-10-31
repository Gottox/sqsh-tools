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
 * @file         directory_iterator.h
 */

#include "../context/inode_context.h"
#include "../context/metablock_stream_context.h"
#include "../utils.h"
#include <stdint.h>

#ifndef SQSH_DIRECTORY_CONTEXT_H

#define SQSH_DIRECTORY_CONTEXT_H

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
#endif /* end of include guard SQSH_DIRECTORY_CONTEXT_H */