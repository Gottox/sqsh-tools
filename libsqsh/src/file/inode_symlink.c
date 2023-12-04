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
 * @file         symlink_file.c
 */

#include <sqsh_file_private.h>

#include <cextras/memory.h>
#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>
#include <sqsh_table.h>

#include <sqsh_data_private.h>
#include <sqsh_tree_private.h>
#include <stdint.h>

static uint64_t inode_symlink_size(const struct SqshDataInode *inode);
static uint64_t inode_symlink_ext_size(const struct SqshDataInode *inode);

/* payload_size */
static size_t
inode_symlink_payload_size(
		const struct SqshDataInode *inode, const struct SqshArchive *archive) {
	(void)archive;
	return inode_symlink_size(inode);
}

static size_t
inode_symlink_ext_payload_size(
		const struct SqshDataInode *inode, const struct SqshArchive *archive) {
	(void)archive;
	return inode_symlink_ext_size(inode);
}

/* hard_link_count */
static uint32_t
inode_symlink_hard_link_count(const struct SqshDataInode *inode) {
	return sqsh__data_inode_symlink_hard_link_count(
			sqsh__data_inode_symlink(inode));
}

static uint32_t
inode_symlink_ext_hard_link_count(const struct SqshDataInode *inode) {
	return sqsh__data_inode_symlink_ext_hard_link_count(
			sqsh__data_inode_symlink_ext(inode));
}

/* size */
static uint64_t
inode_symlink_size(const struct SqshDataInode *inode) {
	return sqsh__data_inode_symlink_target_size(
			sqsh__data_inode_symlink(inode));
}

static uint64_t
inode_symlink_ext_size(const struct SqshDataInode *inode) {
	return sqsh__data_inode_symlink_ext_target_size(
			sqsh__data_inode_symlink_ext(inode));
}

static const char *
inode_symlink_target_path(const struct SqshDataInode *inode) {
	return (const char *)sqsh__data_inode_symlink_ext_target_path(
			sqsh__data_inode_symlink_ext(inode));
}

static const char *
inode_symlink_ext_target_path(const struct SqshDataInode *inode) {
	return (const char *)sqsh__data_inode_symlink_ext_target_path(
			sqsh__data_inode_symlink_ext(inode));
}

uint32_t
inode_symlink_ext_xattr_index(const struct SqshDataInode *inode) {
	return sqsh__data_inode_symlink_ext_xattr_idx(
			sqsh__data_inode_symlink_ext(inode));
}

const struct SqshInodeImpl sqsh__inode_symlink_impl = {
		.header_size = sizeof(struct SqshDataInodeSymlink),
		.payload_size = inode_symlink_payload_size,

		.hard_link_count = inode_symlink_hard_link_count,
		.size = inode_symlink_size,

		.symlink_target_path = inode_symlink_target_path,
};

const struct SqshInodeImpl sqsh__inode_symlink_ext_impl = {
		.header_size = sizeof(struct SqshDataInodeSymlinkExt),
		.payload_size = inode_symlink_ext_payload_size,

		.hard_link_count = inode_symlink_ext_hard_link_count,
		.size = inode_symlink_ext_size,

		.blocks_start = NULL,
		.block_size_info = NULL,
		.fragment_block_index = NULL,
		.fragment_block_offset = NULL,

		.directory_block_start = NULL,
		.directory_block_offset = NULL,
		.directory_parent_inode = NULL,

		.symlink_target_path = inode_symlink_ext_target_path,

		.device_id = NULL,

		.xattr_index = inode_symlink_ext_xattr_index,
};
