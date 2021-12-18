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
#include "metablock_stream_context.h"
#include <stdint.h>
#include <sys/types.h>

#ifndef HSQS_INODE_CONTEXT_H

#define HSQS_INODE_CONTEXT_H

#define HSQS_INODE_NO_FRAGMENT 0xFFFFFFFF
#define HSQS_INODE_NO_XATTR 0xFFFFFFFF

struct Hsqs;

struct HsqsSuperblockContext;
struct HsqsInode;
struct HsqsInodeTable;
struct HsqsDirectoryIterator;
struct HsqsXattrTableIterator;

enum HsqsInodeContextType {
	HSQS_INODE_TYPE_UNKNOWN = -1,
	// avoid overlapping with the types in ../data/inode.h
	HSQS_INODE_TYPE_DIRECTORY = 1 + (1 << 8),
	HSQS_INODE_TYPE_FILE,
	HSQS_INODE_TYPE_SYMLINK,
	HSQS_INODE_TYPE_BLOCK,
	HSQS_INODE_TYPE_CHAR,
	HSQS_INODE_TYPE_FIFO,
	HSQS_INODE_TYPE_SOCKET,
};

struct HsqsInodeContext {
	struct HsqsInode *inode;
	struct HsqsMetablockStreamContext metablock;
	struct Hsqs *hsqs;
};

struct HsqsInodeDirectoryIndexIterator {
	struct HsqsInodeContext *inode;
	const struct HsqsInodeDirectoryIndex *indices;
	size_t remaining_entries;
	off_t current_offset;
	off_t next_offset;
};

HSQS_NO_UNUSED int hsqs_inode_load(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs, uint64_t inode_ref);
int hsqs_inode_load_root(struct HsqsInodeContext *inode, struct Hsqs *hsqs);
HSQS_NO_UNUSED int hsqs_inode_load_by_inode_number(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs,
		uint64_t inode_number);

HSQS_NO_UNUSED uint32_t
hsqs_inode_hard_link_count(const struct HsqsInodeContext *inode);

uint64_t hsqs_inode_file_size(const struct HsqsInodeContext *inode);
uint16_t hsqs_inode_permission(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_number(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_modified_time(const struct HsqsInodeContext *inode);
uint64_t hsqs_inode_file_blocks_start(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_file_block_size(
		const struct HsqsInodeContext *inode, uint32_t index);
bool hsqs_inode_file_block_is_compressed(
		const struct HsqsInodeContext *inode, int index);
uint32_t
hsqs_inode_file_fragment_block_index(const struct HsqsInodeContext *inode);
uint32_t
hsqs_inode_file_fragment_block_offset(const struct HsqsInodeContext *inode);
bool hsqs_inode_file_has_fragment(const struct HsqsInodeContext *inode);

enum HsqsInodeContextType hsqs_inode_type(const struct HsqsInodeContext *inode);

const char *hsqs_inode_symlink(const struct HsqsInodeContext *inode);
HSQS_NO_UNUSED int
hsqs_inode_symlink_dup(const struct HsqsInodeContext *inode, char **namebuffer);
uint32_t hsqs_inode_symlink_size(const struct HsqsInodeContext *inode);

uint32_t hsqs_inode_device_id(const struct HsqsInodeContext *inode);

uint32_t hsqs_inode_uid(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_gid(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_xattr_index(const struct HsqsInodeContext *inode);
uint32_t hsqs_inode_xattr_index(const struct HsqsInodeContext *inode);
HSQS_NO_UNUSED int hsqs_inode_xattr_iterator(
		const struct HsqsInodeContext *inode,
		struct HsqsXattrTableIterator *iterator);

int hsqs_inode_cleanup(struct HsqsInodeContext *inode);

HSQS_NO_UNUSED int hsqs_inode_directory_iterator_init(
		struct HsqsInodeDirectoryIndexIterator *iterator,
		struct HsqsInodeContext *inode);
HSQS_NO_UNUSED int hsqs_inode_directory_index_iterator_next(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_index(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_start(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_name_size(
		struct HsqsInodeDirectoryIndexIterator *iterator);
const char *hsqs_inode_directory_index_iterator_name(
		struct HsqsInodeDirectoryIndexIterator *iterator);

HSQS_NO_UNUSED int hsqs_inode_directory_index_iterator_clean(
		struct HsqsInodeDirectoryIndexIterator *iterator);

void
hsqs_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset);
HSQS_NO_UNUSED uint64_t
hsqs_inode_ref_from_block(uint32_t block_index, uint16_t offset);

#endif /* end of include guard HSQS_INODE_CONTEXT_H */
