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
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include "../utils.h"
#include "inode_context.h"
#include <stdint.h>

#ifndef SQUASH_DIRECTORY_CONTEXT_H

#define SQUASH_DIRECTORY_CONTEXT_H

struct SquashInodeContext;
struct SquashSuperblockContext;

struct SquashDirectoryContext {
	const struct SquashSuperblockContext *superblock;
	struct SquashInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;
};

struct SquashDirectoryIterator {
	const struct SquashDirectoryFragment *fragments;
	struct SquashDirectoryContext *directory;
	struct SquashMetablockContext extract;
	size_t remaining_entries;
	off_t current_fragment_offset;
	off_t next_offset;
	off_t current_offset;
};

SQUASH_NO_UNUSED int squash_directory_init(
		struct SquashDirectoryContext *directory,
		const struct SquashSuperblockContext *superblock,
		struct SquashInodeContext *inode);
SQUASH_NO_UNUSED int squash_directory_iterator_init(
		struct SquashDirectoryIterator *iterator,
		struct SquashDirectoryContext *directory);
SQUASH_NO_UNUSED int squash_directory_iterator_next(
		struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_lookup(
		struct SquashDirectoryIterator *iterator, const char *name,
		const size_t name_len);
int squash_directory_iterator_name_size(
		const struct SquashDirectoryIterator *iterator);
uint64_t squash_directory_iterator_inode_ref(
		const struct SquashDirectoryIterator *iterator);
enum SquashInodeContextType squash_directory_iterator_inode_type(
		const struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_inode_load(
		const struct SquashDirectoryIterator *iterator,
		struct SquashInodeContext *inode);
const char *squash_directory_iterator_name(
		const struct SquashDirectoryIterator *iterator);
SQUASH_NO_UNUSED int squash_directory_iterator_name_dup(
		const struct SquashDirectoryIterator *iterator, char **name_buffer);
int squash_directory_iterator_cleanup(struct SquashDirectoryIterator *iterator);

int squash_directory_cleanup(struct SquashDirectoryContext *directory);
#endif /* end of include guard SQUASH_DIRECTORY_CONTEXT_H */
