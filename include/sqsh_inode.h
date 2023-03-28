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
 * @file         sqsh_inode.h
 */

#ifndef SQSH_INODE_H
#define SQSH_INODE_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// inode/inode.c

#define SQSH_INODE_NO_FRAGMENT 0xFFFFFFFF
#define SQSH_INODE_NO_XATTR 0xFFFFFFFF

enum SqshInodeType {
	SQSH_INODE_TYPE_BASIC_DIRECTORY = 1,
	SQSH_INODE_TYPE_BASIC_FILE = 2,
	SQSH_INODE_TYPE_BASIC_SYMLINK = 3,
	SQSH_INODE_TYPE_BASIC_BLOCK = 4,
	SQSH_INODE_TYPE_BASIC_CHAR = 5,
	SQSH_INODE_TYPE_BASIC_FIFO = 6,
	SQSH_INODE_TYPE_BASIC_SOCKET = 7,
	SQSH_INODE_TYPE_EXTENDED_DIRECTORY = 8,
	SQSH_INODE_TYPE_EXTENDED_FILE = 9,
	SQSH_INODE_TYPE_EXTENDED_SYMLINK = 10,
	SQSH_INODE_TYPE_EXTENDED_BLOCK = 11,
	SQSH_INODE_TYPE_EXTENDED_CHAR = 12,
	SQSH_INODE_TYPE_EXTENDED_FIFO = 13,
	SQSH_INODE_TYPE_EXTENDED_SOCKET = 14,
};

enum SqshInodeContextType {
	SQSH_INODE_TYPE_UNKNOWN = -1,
	// avoid overlapping with the types in inode_data.h
	SQSH_INODE_TYPE_DIRECTORY = 1 + (1 << 8),
	SQSH_INODE_TYPE_FILE,
	SQSH_INODE_TYPE_SYMLINK,
	SQSH_INODE_TYPE_BLOCK,
	SQSH_INODE_TYPE_CHAR,
	SQSH_INODE_TYPE_FIFO,
	SQSH_INODE_TYPE_SOCKET,
};

/**
 * @memberof SqshInode
 * @brief Initializes an inode context in heap
 *
 * @param sqsh The sqsh context to use.
 * @param inode_ref The inode reference to initialize the context with.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return a pointer to the sqsh context or NULL if an error occurred.
 */
SQSH_NO_UNUSED struct SqshInode *
sqsh_inode_new(struct SqshArchive *sqsh, uint64_t inode_ref, int *err);

/**
 * @memberof SqshInode
 * @brief returns whether the inode is an extended structure.
 *
 * @param[in] context The inode context.
 *
 * @return true if the inode is an extended structure.
 */
bool sqsh_inode_is_extended(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the inode hard link count.
 *
 * @param[in] context The inode context.
 *
 * @return the amount of hard links to the inode.
 */
uint32_t sqsh_inode_hard_link_count(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the file size. 0 if the file has no size.
 *
 * @param[in] context The inode context.
 *
 * @return the inode type.
 */
uint64_t sqsh_inode_file_size(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the permissions of the inode.
 *
 * @param[in] context The inode context.
 *
 * @return the permissions of the inode.
 */
uint16_t sqsh_inode_permission(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the inode number.
 *
 * @param[in] context The inode context.
 *
 * @return the inode number.
 */
uint32_t sqsh_inode_number(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the inode modification time.
 *
 * @param[in] context The inode context.
 *
 * @return the inode modification time.
 */
uint32_t sqsh_inode_modified_time(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the start block of the file content. This is only
 * internally used and will be used while retrieving the file content.
 *
 * @param[in] context The inode context.
 *
 * @return the start block of the file content or UINT64_MAX if the inode
 * is not a file.
 */
uint64_t sqsh_inode_file_blocks_start(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter for the amount of blocks of the file content. This is only
 * internally used and will be used while retrieving the file content.
 *
 * @param[in] context The inode context.
 *
 * @return the amount of blocks of the file content. If the inode is not a
 * file 0, UINT32_MAX will be returned.
 */
uint32_t sqsh_inode_file_block_count(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief Getter the size of a block of the file content. This is only
 * internally used and will be used while retrieving the file content.
 *
 * @param[in] context The inode context.
 * @param index The index of the block.
 *
 * @return the size of the block with the index.
 */
uint32_t sqsh_inode_file_block_size(
		const struct SqshInode *context, uint32_t index);

/**
 * @memberof SqshInode
 * @brief Checks whether a certain block is compressed.
 *
 * @param[in] context The inode context.
 * @param index The index of the block.
 *
 * @return true if the block is compressed, false otherwise.
 */
bool sqsh_inode_file_block_is_compressed(
		const struct SqshInode *context, int index);

/**
 * @memberof SqshInode
 * @brief retrieve the fragment block index. This is only internally used
 *
 * and will be used while retrieving the file content.
 * @param[in] context The inode context.
 *
 * @return the fragment block index.
 */
uint32_t
sqsh_inode_file_fragment_block_index(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief retrieve the fragment block offset. This is only internally used
 * and will be used while retrieving the file content.
 *
 * @param[in] context The inode context.
 *
 * @return the offset inside of the fragment block.
 */
uint32_t
sqsh_inode_file_fragment_block_offset(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief retrieve the directory block start. This is only internally used
 * and will be used while iterating over the directory entries.
 *
 * @param[in] context The inode context.
 *
 * @return the directory block start.
 */
uint32_t
sqsh_inode_directory_block_start(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief retrieve the directory block offset. This is only internally used
 * and will be used while iterating over the directory entries.
 *
 * @param[in] context The inode context.
 *
 * @return the directory block offset.
 */
uint32_t
sqsh_inode_directory_block_offset(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns true if the inode has a fragment block.
 *
 * @param[in] context The inode context.
 *
 * @return true if the inode has a fragment block, false otherwise.
 */
bool sqsh_inode_file_has_fragment(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns the type of the inode.
 *
 * @param[in] context The inode context.
 *
 * @return the type of the inode.
 */
enum SqshInodeContextType
sqsh_inode_type(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns the target of a symbolic link. Be aware that the returned
 * value is not zero terminated. If you need a zero terminated string use
 * sqsh_inode_symlink_dup().
 *
 * @param[in] context The inode context.
 *
 * @return the name of the inode or NULL if the inode is not a symbolic link.
 */
const char *sqsh_inode_symlink(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief sets a heap allocated, zero terminated string of the target of a
 * symbolic link.
 *
 * @param[in] context The inode context.
 * @param namebuffer a pointer that will be set to the allocated string.
 *
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_symlink_dup(
		const struct SqshInode *context, char **namebuffer);

/**
 * @memberof SqshInode
 * @brief returns the length of the target of a symbolic link in bytes.
 *
 * @param[in] context The inode context.
 *
 * @return the length of the target of a symbolic link in bytes or 0 if the
 * inode is not a symbolic link.
 */
uint32_t sqsh_inode_symlink_size(const struct SqshInode *context);

/**
 * @memberof SqshInode
 *
 * @brief returns the device id of the device inode.
 *
 * @param[in] context The inode context.
 *
 * @return the name of the inode or 0 if the inode is not a device.
 */
uint32_t sqsh_inode_device_id(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns the uid of the inode.
 *
 * @param[in] context The inode context.
 *
 * @return the uid of the inode.
 */
uint32_t sqsh_inode_uid(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns the gid of the inode.
 *
 * @param[in] context The inode context.
 *
 * @return the gid of the inode.
 */
uint32_t sqsh_inode_gid(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns the reference to this inode.
 *
 * @param[in] context The inode context.
 *
 * @return the reference to this inode.
 */
uint64_t sqsh_inode_ref(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief returns index of the extended attribute inside of the xattr table.
 *
 * @param[in] context The inode context.
 *
 * @return the index of the extended attribute inside of the xattr table.
 */
uint32_t sqsh_inode_xattr_index(const struct SqshInode *context);

/**
 * @memberof SqshInode
 * @brief cleans up an inode context and frees the memory.
 *
 * @param[in] context The inode context.
 *
 * @return int 0 on success, less than 0 on error.
 */
int sqsh_inode_free(struct SqshInode *context);

#ifdef __cplusplus
}
#endif
#endif // SQSH_INODE_H
