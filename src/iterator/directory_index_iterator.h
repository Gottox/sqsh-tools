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
 * @file         directory_index_iterator.h
 */

#include "../utils.h"

#ifndef DIRECTORY_INDEX_ITERATOR_H

#define DIRECTORY_INDEX_ITERATOR_H

struct SqshInodeContext;

struct SqshInodeDirectoryIndexIterator {
	struct SqshInodeContext *inode;
	size_t remaining_entries;
	sqsh_index_t current_offset;
	sqsh_index_t next_offset;
};

HSQS_NO_UNUSED int sqsh_inode_directory_index_iterator_init(
		struct SqshInodeDirectoryIndexIterator *iterator,
		struct SqshInodeContext *inode);
HSQS_NO_UNUSED int sqsh_inode_directory_index_iterator_next(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_index(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_start(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh_inode_directory_index_iterator_name_size(
		struct SqshInodeDirectoryIndexIterator *iterator);
const char *sqsh_inode_directory_index_iterator_name(
		struct SqshInodeDirectoryIndexIterator *iterator);

HSQS_NO_UNUSED int sqsh_inode_directory_index_iterator_clean(
		struct SqshInodeDirectoryIndexIterator *iterator);

#endif /* end of include guard DIRECTORY_INDEX_ITERATOR_H */
