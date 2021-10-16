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
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "../utils.h"
#include "metablock_context.h"
#include <stdint.h>
#include <sys/types.h>

#ifndef SQUASH_INODE_CONTEXT_H

#define SQUASH_INODE_CONTEXT_H

#define SQUASH_INODE_NO_FRAGMENT 0xFFFFFFFF

struct SquashSuperblockContext;
struct SquashInode;
struct SquashInodeTable;
struct SquashDirectoryIterator;

enum SquashInodeContextType {
	SQUASH_INODE_TYPE_UNKNOWN = -1,
	// avoid overlapping with the types in ../data/inode.h
	SQUASH_INODE_TYPE_DIRECTORY = 1 + (1 << 8),
	SQUASH_INODE_TYPE_FILE,
	SQUASH_INODE_TYPE_SYMLINK,
	SQUASH_INODE_TYPE_BLOCK,
	SQUASH_INODE_TYPE_CHAR,
	SQUASH_INODE_TYPE_FIFO,
	SQUASH_INODE_TYPE_SOCKET,
};

struct SquashInodeContext {
	struct SquashInode *inode;
	struct SquashMetablockContext extract;
	uint32_t datablock_block_size;
};

struct SquashInodeDirectoryIndexIterator {
	struct SquashInodeContext *inode;
	const struct SquashInodeDirectoryIndex *indices;
	size_t remaining_entries;
	off_t current_offset;
	off_t next_offset;
};

SQUASH_NO_UNUSED int squash_inode_load(
		struct SquashInodeContext *inode,
		const struct SquashSuperblockContext *superblock, uint64_t inode_ref);

SQUASH_NO_UNUSED uint32_t
squash_inode_hard_link_count(const struct SquashInodeContext *inode);

uint64_t squash_inode_file_size(const struct SquashInodeContext *inode);
uint16_t squash_inode_permission(const struct SquashInodeContext *inode);
uint32_t squash_inode_modified_time(const struct SquashInodeContext *inode);
uint64_t squash_inode_file_blocks_start(const struct SquashInodeContext *inode);
// TODO: Find right datatype for index
uint32_t
squash_inode_file_block_size(const struct SquashInodeContext *inode, int index);
bool squash_inode_file_block_is_compressed(
		const struct SquashInodeContext *inode, int index);
uint32_t
squash_inode_file_fragment_block_index(const struct SquashInodeContext *inode);
uint32_t
squash_inode_file_fragment_block_offset(const struct SquashInodeContext *inode);

enum SquashInodeContextType
squash_inode_type(const struct SquashInodeContext *inode);

const char *squash_inode_symlink(const struct SquashInodeContext *inode);
SQUASH_NO_UNUSED int squash_inode_symlink_dup(
		const struct SquashInodeContext *inode, char **namebuffer);
uint32_t squash_inode_symlink_size(const struct SquashInodeContext *inode);

int squash_inode_cleanup(struct SquashInodeContext *inode);

SQUASH_NO_UNUSED int squash_inode_directory_iterator_init(
		struct SquashInodeDirectoryIndexIterator *iterator,
		struct SquashInodeContext *inode);
SQUASH_NO_UNUSED int squash_inode_directory_index_iterator_next(
		struct SquashInodeDirectoryIndexIterator *iterator);
uint32_t squash_inode_directory_index_iterator_index(
		struct SquashInodeDirectoryIndexIterator *iterator);
uint32_t squash_inode_directory_index_iterator_start(
		struct SquashInodeDirectoryIndexIterator *iterator);
uint32_t squash_inode_directory_index_iterator_name_size(
		struct SquashInodeDirectoryIndexIterator *iterator);
const char *squash_inode_directory_index_iterator_name(
		struct SquashInodeDirectoryIndexIterator *iterator);

SQUASH_NO_UNUSED int squash_inode_directory_index_iterator_clean(
		struct SquashInodeDirectoryIndexIterator *iterator);

void squash_inode_ref_to_block(
		uint64_t ref, uint32_t *block_index, uint16_t *offset);
SQUASH_NO_UNUSED uint64_t
squash_inode_ref_from_block(uint32_t block_index, uint16_t offset);

#endif /* end of include guard SQUASH_INODE_CONTEXT_H */
