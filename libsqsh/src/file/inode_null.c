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
 * @file         inode_context.c
 */

#include <sqsh_file_private.h>

#include <cextras/memory.h>
#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>
#include <sqsh_table.h>

#include <sqsh_data_private.h>
#include <sqsh_tree_private.h>

uint32_t
sqsh__file_inode_null_directory_block_start(const struct SqshDataInode *inode) {
	(void)inode;
	return UINT32_MAX;
}

uint16_t
sqsh__file_inode_null_directory_block_offset(
		const struct SqshDataInode *inode) {
	(void)inode;
	return UINT16_MAX;
}

uint32_t
sqsh__file_inode_null_directory_parent_inode(
		const struct SqshDataInode *inode) {
	(void)inode;
	return UINT32_MAX;
}

uint64_t
sqsh__file_inode_null_blocks_start(const struct SqshDataInode *inode) {
	(void)inode;
	return UINT64_MAX;
}

uint32_t
sqsh__file_inode_null_block_size_info(
		const struct SqshDataInode *inode, sqsh_index_t index) {
	(void)inode;
	(void)index;
	return UINT32_MAX;
}

uint32_t
sqsh__file_inode_null_fragment_block_index(const struct SqshDataInode *inode) {
	(void)inode;
	return UINT32_MAX;
}

uint32_t
sqsh__file_inode_null_fragment_block_offset(const struct SqshDataInode *inode) {
	(void)inode;
	return UINT32_MAX;
}

const char *
sqsh__file_inode_null_symlink_target_path(const struct SqshDataInode *inode) {
	(void)inode;
	return NULL;
}

uint32_t
sqsh__file_inode_null_symlink_target_size(const struct SqshDataInode *inode) {
	(void)inode;
	return 0;
}

uint32_t
sqsh__file_inode_null_device_id(const struct SqshDataInode *inode) {
	(void)inode;
	return UINT32_MAX;
}

uint32_t
sqsh__file_inode_null_xattr_index(const struct SqshDataInode *inode) {
	(void)inode;
	return SQSH_INODE_NO_XATTR;
}
