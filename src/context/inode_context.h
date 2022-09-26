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
 * @file         inode.h
 */

#include "../utils.h"
#include "metablock_stream_context.h"
#include <stdint.h>
#include <sys/types.h>

#ifndef HSQS_INODE_CONTEXT_H

#define HSQS_INODE_CONTEXT_H

#define HSQS_INODE_NO_FRAGMENT 0xFFFFFFFF
#define HSQS_INODE_NO_XATTR 0xFFFFFFFF

struct Sqsh;

struct SqshSuperblockContext;
struct SqshInode;
struct SqshInodeTable;
struct SqshDirectoryIterator;
struct SqshXattrIterator;

enum SqshInodeContextType {
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

struct SqshInodeContext {
	struct SqshMetablockStreamContext metablock;
	struct Sqsh *sqsh;
};

HSQS_NO_UNUSED int sqsh_inode_load_by_ref(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_ref);
int sqsh_inode_load_root(struct SqshInodeContext *context, struct Sqsh *sqsh);
HSQS_NO_UNUSED int sqsh_inode_load_by_inode_number(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_number);
HSQS_NO_UNUSED int sqsh_inode_load_by_path(
		struct SqshInodeContext *context, struct Sqsh *sqsh, const char *path);

bool sqsh_inode_is_extended(const struct SqshInodeContext *context);
uint32_t sqsh_inode_hard_link_count(const struct SqshInodeContext *context);
uint64_t sqsh_inode_file_size(const struct SqshInodeContext *context);
uint16_t sqsh_inode_permission(const struct SqshInodeContext *context);
uint32_t sqsh_inode_number(const struct SqshInodeContext *context);
uint32_t sqsh_inode_modified_time(const struct SqshInodeContext *context);
uint64_t sqsh_inode_file_blocks_start(const struct SqshInodeContext *context);
uint32_t sqsh_inode_file_block_count(const struct SqshInodeContext *context);
uint32_t sqsh_inode_file_block_size(
		const struct SqshInodeContext *context, uint32_t index);
bool sqsh_inode_file_block_is_compressed(
		const struct SqshInodeContext *context, int index);
uint32_t
sqsh_inode_file_fragment_block_index(const struct SqshInodeContext *context);
uint32_t
sqsh_inode_file_fragment_block_offset(const struct SqshInodeContext *context);
uint32_t
sqsh_inode_directory_block_start(const struct SqshInodeContext *context);
uint32_t
sqsh_inode_directory_block_offset(const struct SqshInodeContext *context);
bool sqsh_inode_file_has_fragment(const struct SqshInodeContext *context);

enum SqshInodeContextType
sqsh_inode_type(const struct SqshInodeContext *context);

const char *sqsh_inode_symlink(const struct SqshInodeContext *context);
HSQS_NO_UNUSED int sqsh_inode_symlink_dup(
		const struct SqshInodeContext *context, char **namebuffer);
uint32_t sqsh_inode_symlink_size(const struct SqshInodeContext *context);

uint32_t sqsh_inode_device_id(const struct SqshInodeContext *context);

uint32_t sqsh_inode_uid(const struct SqshInodeContext *context);
uint32_t sqsh_inode_gid(const struct SqshInodeContext *context);
uint32_t sqsh_inode_xattr_index(const struct SqshInodeContext *context);
HSQS_NO_UNUSED int sqsh_inode_xattr_iterator(
		const struct SqshInodeContext *context,
		struct SqshXattrIterator *iterator);
int sqsh_inode_cleanup(struct SqshInodeContext *context);
void
sqsh_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset);
HSQS_NO_UNUSED uint64_t
sqsh_inode_ref_from_block(uint32_t block_index, uint16_t offset);

#endif /* end of include guard HSQS_INODE_CONTEXT_H */
