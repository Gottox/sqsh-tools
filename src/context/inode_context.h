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
 * @file         inode_context.h
 */

#include "../utils.h"
#include "metablock_stream_context.h"
#include <stdint.h>
#include <sys/types.h>

#ifndef SQSH_INODE_CONTEXT_H

#define SQSH_INODE_CONTEXT_H

#define SQSH_INODE_NO_FRAGMENT 0xFFFFFFFF
#define SQSH_INODE_NO_XATTR 0xFFFFFFFF

struct Sqsh;

struct SqshSuperblockContext;
struct SqshInode;
struct SqshInodeTable;
struct SqshDirectoryIterator;
struct SqshXattrIterator;

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
	// avoid overlapping with the types in ../data/inode_data.h
	SQSH_INODE_TYPE_DIRECTORY = 1 + (1 << 8),
	SQSH_INODE_TYPE_FILE,
	SQSH_INODE_TYPE_SYMLINK,
	SQSH_INODE_TYPE_BLOCK,
	SQSH_INODE_TYPE_CHAR,
	SQSH_INODE_TYPE_FIFO,
	SQSH_INODE_TYPE_SOCKET,
};

/**
 * @brief Inode context.
 */
struct SqshInodeContext {
	struct SqshMetablockStreamContext metablock;
	struct Sqsh *sqsh;
};

/**
 * @brief Initialize the inode context from a inode reference. inode references
 * @memberof SqshInodeContext
 * are descriptors of the physical location of an inode inside the inode table.
 * They are diffrent from the inode number. In doubt use the inode number.
 *
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param inode_ref The inode reference.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_ref(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_ref);
/**
 * @brief Initialize the inode context of the root directory.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_inode_init_root(struct SqshInodeContext *context, struct Sqsh *sqsh);
/**
 * @brief Initialize the inode context from an inode number.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param inode_number The inode number.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_inode_number(
		struct SqshInodeContext *context, struct Sqsh *sqsh,
		uint64_t inode_number);
/**
 * @brief Initialize the inode context from a path.
 * @memberof SqshInodeContext
 * @param context The inode context to initialize.
 * @param sqsh The sqsh context.
 * @param path The path the file or directory.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_init_by_path(
		struct SqshInodeContext *context, struct Sqsh *sqsh, const char *path);

/**
 * @brief returns whether the inode is an extended structure.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return true if the inode is an extended structure.
 */
bool sqsh_inode_is_extended(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode hard link count.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the amount of hard links to the inode.
 */
uint32_t sqsh_inode_hard_link_count(const struct SqshInodeContext *context);
/**
 * @brief Getter for the file size. 0 if the file has no size.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode type.
 */
uint64_t sqsh_inode_file_size(const struct SqshInodeContext *context);
/**
 * @brief Getter for the permissions of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the permissions of the inode.
 */
uint16_t sqsh_inode_permission(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode number.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode number.
 */
uint32_t sqsh_inode_number(const struct SqshInodeContext *context);
/**
 * @brief Getter for the inode modification time.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the inode modification time.
 */
uint32_t sqsh_inode_modified_time(const struct SqshInodeContext *context);
/**
 * @brief Getter for the start block of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the start block of the file content or UINT64_MAX if the inode
 * is not a file.
 */
uint64_t sqsh_inode_file_blocks_start(const struct SqshInodeContext *context);
/**
 * @brief Getter for the amount of blocks of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the amount of blocks of the file content. If the inode is not a
 * file 0, UINT32_MAX will be returned.
 */
uint32_t sqsh_inode_file_block_count(const struct SqshInodeContext *context);
/**
 * @brief Getter the size of a block of the file content. This is only
 * @memberof SqshInodeContext
 * internally used and will be used while retrieving the file content.
 * @param context The inode context.
 * @param index The index of the block.
 * @return the size of the block with the index.
 */
uint32_t sqsh_inode_file_block_size(
		const struct SqshInodeContext *context, uint32_t index);
/**
 * @brief Checks whether a certain block is compressed.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @param index The index of the block.
 * @return true if the block is compressed, false otherwise.
 */
bool sqsh_inode_file_block_is_compressed(
		const struct SqshInodeContext *context, int index);
/**
 * @brief retrieve the fragment block index. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the fragment block index.
 */
uint32_t
sqsh_inode_file_fragment_block_index(const struct SqshInodeContext *context);
/**
 * @brief retrieve the fragment block offset. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while retrieving the file content.
 * @param context The inode context.
 * @return the offset inside of the fragment block.
 */
uint32_t
sqsh_inode_file_fragment_block_offset(const struct SqshInodeContext *context);
/**
 * @brief retrieve the directory block start. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while iterating over the directory entries.
 * @param context The inode context.
 * @return the directory block start.
 */
uint32_t
sqsh_inode_directory_block_start(const struct SqshInodeContext *context);
/**
 * @brief retrieve the directory block offset. This is only internally used
 * @memberof SqshInodeContext
 * and will be used while iterating over the directory entries.
 * @param context The inode context.
 * @return the directory block offset.
 */
uint32_t
sqsh_inode_directory_block_offset(const struct SqshInodeContext *context);
/**
 * @brief returns true if the inode has a fragment block.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return true if the inode has a fragment block, false otherwise.
 */
bool sqsh_inode_file_has_fragment(const struct SqshInodeContext *context);

/**
 * @brief returns the type of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the type of the inode.
 */
enum SqshInodeContextType
sqsh_inode_type(const struct SqshInodeContext *context);

/**
 * @brief returns the target of a symbolic link. Be aware that the returned
 * @memberof SqshInodeContext
 * value is not zero terminated. If you need a zero terminated string use
 * sqsh_inode_symlink_dup().
 * @param context The inode context.
 * @return the name of the inode or NULL if the inode is not a symbolic link.
 */
const char *sqsh_inode_symlink(const struct SqshInodeContext *context);
/**
 * @brief sets a heap allocated, zero terminated string of the target of a
 * @memberof SqshInodeContext
 * symbolic link.
 * @param context The inode context.
 * @param namebuffer a pointer that will be set to the allocated string.
 * @return int 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh_inode_symlink_dup(
		const struct SqshInodeContext *context, char **namebuffer);
/**
 * @brief returns the length of the target of a symbolic link in bytes.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the length of the target of a symbolic link in bytes or 0 if the
 * inode is not a symbolic link.
 */
uint32_t sqsh_inode_symlink_size(const struct SqshInodeContext *context);

/**
 * @brief returns the device id of the device inode.
 * @param context The inode context.
 * @return the name of the inode or 0 if the inode is not a device.
 */
uint32_t sqsh_inode_device_id(const struct SqshInodeContext *context);

/**
 * @brief returns the uid of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the uid of the inode.
 */
uint32_t sqsh_inode_uid(const struct SqshInodeContext *context);
/**
 * @brief returns the gid of the inode.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the gid of the inode.
 */
uint32_t sqsh_inode_gid(const struct SqshInodeContext *context);
/**
 * @brief returns index of the extended attribute inside of the xattr table.
 * @memberof SqshInodeContext
 * @param context The inode context.
 * @return the index of the extended attribute inside of the xattr table.
 */
uint32_t sqsh_inode_xattr_index(const struct SqshInodeContext *context);
SQSH_NO_UNUSED int sqsh_inode_xattr_iterator(
		const struct SqshInodeContext *context,
		struct SqshXattrIterator *iterator);
int sqsh_inode_cleanup(struct SqshInodeContext *context);
/**
 * @brief converts an inode reference into a block index and a block offset
 * @memberof SqshInodeContext
 * @param ref The inode reference.
 * @param block_index a pointer where the block index will be stored.
 * @param offset a pointer where the block offset will be stored.
 */
void
sqsh_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset);
/**
 * @brief converts a block index and a block offset into an inode reference.
 * @memberof SqshInodeContext
 * @param block_index The block index.
 * @param offset The block offset.
 * @return the inode reference.
 */
SQSH_NO_UNUSED uint64_t
sqsh_inode_ref_from_block(uint32_t block_index, uint16_t offset);

#endif /* end of include guard SQSH_INODE_CONTEXT_H */
