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

#include "inode_context.h"
#include "../data/datablock_internal.h"
#include "../data/inode.h"
#include "../iterator/xattr_iterator.h"

#include "../error.h"
#include "../hsqs.h"
#include "../iterator/directory_iterator.h"
#include "../utils.h"
#include "superblock_context.h"
#include <stdint.h>
#include <string.h>

static const char *
path_find_next_segment(const char *segment) {
	segment = strchr(segment, '/');
	if (segment == NULL) {
		return NULL;
	} else {
		return segment + 1;
	}
}

size_t
path_get_segment_len(const char *segment) {
	const char *next_segment = path_find_next_segment(segment);
	if (next_segment == NULL) {
		return strlen(segment);
	} else {
		return next_segment - segment - 1;
	}
}

static int
path_segments_count(const char *path) {
	int count = 0;
	for (; path; path = path_find_next_segment(path)) {
		count++;
	}

	return count;
}

static int
path_find_inode_ref(
		uint64_t *target, uint64_t dir_ref, struct Hsqs *hsqs, const char *name,
		const size_t name_len) {
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryIterator iter = {0};
	int rv = 0;
	rv = hsqs_inode_load_by_ref(&inode, hsqs, dir_ref);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_iterator_init(&iter, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_iterator_lookup(&iter, name, name_len);
	if (rv < 0) {
		goto out;
	}

	*target = hsqs_directory_iterator_inode_ref(&iter);

out:
	hsqs_directory_iterator_cleanup(&iter);
	hsqs_inode_cleanup(&inode);
	return rv;
}

static const struct HsqsInode *
get_inode(const struct HsqsInodeContext *inode) {
	return (const struct HsqsInode *)hsqs_metablock_stream_data(
			&inode->metablock);
}

static int
inode_data_more(struct HsqsInodeContext *inode, size_t size) {
	int rv = hsqs_metablock_stream_more(&inode->metablock, size);

	if (rv < 0) {
		return rv;
	}
	return rv;
}

static int
inode_load(struct HsqsInodeContext *context) {
	int rv = 0;
	size_t size = HSQS_SIZEOF_INODE_HEADER;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		size += HSQS_SIZEOF_INODE_DIRECTORY;
		break;
	case HSQS_INODE_TYPE_BASIC_FILE:
		size += HSQS_SIZEOF_INODE_FILE_EXT;
		break;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		size += HSQS_SIZEOF_INODE_SYMLINK;
		break;
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
		size += HSQS_SIZEOF_INODE_DEVICE;
		break;
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_BASIC_SOCKET:
		size += HSQS_SIZEOF_INODE_IPC;
		break;
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		size += HSQS_SIZEOF_INODE_DIRECTORY_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		size += HSQS_SIZEOF_INODE_FILE_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		size += HSQS_SIZEOF_INODE_SYMLINK_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		size += HSQS_SIZEOF_INODE_DEVICE_EXT;
		break;
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		size += HSQS_SIZEOF_INODE_IPC_EXT;
		break;
	}
	rv = inode_data_more(context, size);
	return rv;
}

static const struct HsqsDatablockSize *
get_size_info(const struct HsqsInodeContext *context, int index) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode);
		return &hsqs_data_inode_file_block_sizes(basic_file)[index];
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode);
		return &hsqs_data_inode_file_ext_block_sizes(extended_file)[index];
	}
	// Should never happen
	abort();
}

int
hsqs_inode_load_by_ref(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs, uint64_t inode_ref) {
	uint32_t inode_block;
	uint16_t inode_offset;

	hsqs_inode_ref_to_block(inode_ref, &inode_block, &inode_offset);

	int rv = 0;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);

	rv = hsqs_metablock_stream_init(
			&inode->metablock, hsqs,
			hsqs_superblock_inode_table_start(superblock), ~0);
	if (rv < 0) {
		return rv;
	}
	rv = hsqs_metablock_stream_seek(
			&inode->metablock, inode_block, inode_offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, HSQS_SIZEOF_INODE_HEADER);
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);
	if (rv < 0) {
		return rv;
	}

	inode->hsqs = hsqs;

	return rv;
}

int
hsqs_inode_load_root(struct HsqsInodeContext *inode, struct Hsqs *hsqs) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	uint64_t inode_ref = hsqs_superblock_inode_root_ref(superblock);

	return hsqs_inode_load_by_ref(inode, hsqs, inode_ref);
}

int
hsqs_inode_load_by_inode_number(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs,
		uint64_t inode_number) {
	int rv = 0;
	uint64_t inode_ref;
	struct HsqsTable *export_table;

	rv = hsqs_export_table(hsqs, &export_table);
	if (rv < 0) {
		return rv;
	}

	rv = hsqs_table_get(export_table, inode_number, &inode_ref);
	if (rv < 0) {
		return rv;
	}
	return hsqs_inode_load_by_ref(inode, hsqs, inode_ref);
}

int
hsqs_inode_load_by_path(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs, const char *path) {
	int i;
	int rv = 0;
	int segment_count = path_segments_count(path) + 1;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	const char *segment = path;
	uint64_t *inode_refs = calloc(segment_count, sizeof(uint64_t));
	if (inode_refs == NULL) {
		rv = HSQS_ERROR_MALLOC_FAILED;
		goto out;
	}
	inode_refs[0] = hsqs_superblock_inode_root_ref(superblock);

	for (i = 0; segment; segment = path_find_next_segment(segment)) {
		size_t segment_len = path_get_segment_len(segment);

		if (segment_len == 0 || (segment_len == 1 && segment[0] == '.')) {
			continue;
		} else if (segment_len == 2 && strncmp(segment, "..", 2) == 0) {
			i = MAX(0, i - 1);
			continue;
		} else {
			uint64_t parent_inode_ref = inode_refs[i];
			i++;
			rv = path_find_inode_ref(
					&inode_refs[i], parent_inode_ref, hsqs, segment,
					segment_len);
			if (rv < 0) {
				goto out;
			}
		}
	}

	rv = hsqs_inode_load_by_ref(inode, hsqs, inode_refs[i]);

out:
	free(inode_refs);
	return rv;
}

bool
hsqs_inode_is_extended(const struct HsqsInodeContext *context) {
	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
	case HSQS_INODE_TYPE_BASIC_FILE:
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_BASIC_SOCKET:
		return false;
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
	case HSQS_INODE_TYPE_EXTENDED_FILE:
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return true;
	}
	return false;
}

uint32_t
hsqs_inode_hard_link_count(const struct HsqsInodeContext *context) {
	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		return hsqs_data_inode_directory_hard_link_count(
				hsqs_data_inode_directory(inode));
	case HSQS_INODE_TYPE_BASIC_FILE:
		return 1;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		return hsqs_data_inode_symlink_hard_link_count(
				hsqs_data_inode_symlink(inode));
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
		return hsqs_data_inode_device_hard_link_count(
				hsqs_data_inode_device(inode));
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_BASIC_SOCKET:
		return hsqs_data_inode_ipc_hard_link_count(hsqs_data_inode_ipc(inode));

	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return hsqs_data_inode_directory_ext_hard_link_count(
				hsqs_data_inode_directory_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return hsqs_data_inode_file_ext_hard_link_count(
				hsqs_data_inode_file_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return hsqs_data_inode_symlink_ext_hard_link_count(
				hsqs_data_inode_symlink_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return hsqs_data_inode_device_ext_hard_link_count(
				hsqs_data_inode_device_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return hsqs_data_inode_ipc_ext_hard_link_count(
				hsqs_data_inode_ipc_ext(inode));
	}
	return -HSQS_ERROR_UNKOWN_INODE_TYPE;
}

uint64_t
hsqs_inode_file_size(const struct HsqsInodeContext *context) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;
	const struct HsqsInodeDirectory *basic_dir;
	const struct HsqsInodeDirectoryExt *extended_dir;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = hsqs_data_inode_directory(inode);
		return hsqs_data_inode_directory_file_size(basic_dir);
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = hsqs_data_inode_directory_ext(inode);
		return hsqs_data_inode_directory_ext_file_size(extended_dir);
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode);
		return hsqs_data_inode_file_size(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode);
		return hsqs_data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
hsqs_inode_permission(const struct HsqsInodeContext *inode) {
	return hsqs_data_inode_permissions(get_inode(inode));
}

uint32_t
hsqs_inode_number(const struct HsqsInodeContext *inode) {
	return hsqs_data_inode_number(get_inode(inode));
}

uint32_t
hsqs_inode_modified_time(const struct HsqsInodeContext *inode) {
	return hsqs_data_inode_modified_time(get_inode(inode));
}

uint64_t
hsqs_inode_file_blocks_start(const struct HsqsInodeContext *context) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode);
		return hsqs_data_inode_file_blocks_start(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode);
		return hsqs_data_inode_file_ext_blocks_start(extended_file);
	}
	return UINT64_MAX;
}

bool
hsqs_inode_has_fragment(const struct HsqsInodeContext *context) {
	return hsqs_inode_file_fragment_block_index(context) !=
			HSQS_INODE_NO_FRAGMENT;
}

uint32_t
hsqs_inode_file_block_count(const struct HsqsInodeContext *context) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(context->hsqs);
	uint64_t file_size = hsqs_inode_file_size(context);
	uint32_t block_size = hsqs_superblock_block_size(superblock);

	if (hsqs_inode_has_fragment(context)) {
		return file_size / block_size;
	} else {
		return HSQS_DEVIDE_CEIL(file_size, block_size);
	}
}

uint32_t
hsqs_inode_file_block_size(
		const struct HsqsInodeContext *inode, uint32_t index) {
	const struct HsqsDatablockSize *size_info = get_size_info(inode, index);

	return hsqs_data_datablock_size(size_info);
}

bool
hsqs_inode_file_block_is_compressed(
		const struct HsqsInodeContext *inode, int index) {
	const struct HsqsDatablockSize *size_info = get_size_info(inode, index);

	return hsqs_data_datablock_is_compressed(size_info);
}

uint32_t
hsqs_inode_file_fragment_block_index(const struct HsqsInodeContext *context) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode);
		return hsqs_data_inode_file_fragment_block_index(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode);
		return hsqs_data_inode_file_ext_fragment_block_index(extended_file);
	}
	return HSQS_INODE_NO_FRAGMENT;
}

uint32_t
hsqs_inode_directory_block_start(const struct HsqsInodeContext *context) {
	const struct HsqsInodeDirectory *basic_file;
	const struct HsqsInodeDirectoryExt *extended_file;
	const struct HsqsInode *inode = get_inode(context);

	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		basic_file = hsqs_data_inode_directory(inode);
		return hsqs_data_inode_directory_block_start(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_file = hsqs_data_inode_directory_ext(inode);
		return hsqs_data_inode_directory_ext_block_start(extended_file);
	}
	return UINT32_MAX;
}

uint32_t
hsqs_inode_directory_block_offset(const struct HsqsInodeContext *context) {
	const struct HsqsInodeDirectory *basic_directory;
	const struct HsqsInodeDirectoryExt *extended_directory;
	const struct HsqsInode *inode = get_inode(context);

	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
		basic_directory = hsqs_data_inode_directory(inode);
		return hsqs_data_inode_directory_block_offset(basic_directory);
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_directory = hsqs_data_inode_directory_ext(inode);
		return hsqs_data_inode_directory_ext_block_offset(extended_directory);
	}
	return UINT32_MAX;
}

uint32_t
hsqs_inode_file_fragment_block_offset(const struct HsqsInodeContext *context) {
	const struct HsqsInodeFile *basic_file;
	const struct HsqsInodeFileExt *extended_file;
	const struct HsqsInode *inode = get_inode(context);

	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_FILE:
		basic_file = hsqs_data_inode_file(inode);
		return hsqs_data_inode_file_block_offset(basic_file);
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		extended_file = hsqs_data_inode_file_ext(inode);
		return hsqs_data_inode_file_ext_block_offset(extended_file);
	}
	return HSQS_INODE_NO_FRAGMENT;
}

bool
hsqs_inode_file_has_fragment(const struct HsqsInodeContext *inode) {
	return hsqs_inode_file_fragment_block_index(inode) !=
			HSQS_INODE_NO_FRAGMENT;
}

enum HsqsInodeContextType
hsqs_inode_type(const struct HsqsInodeContext *context) {
	const struct HsqsInode *inode = get_inode(context);

	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_DIRECTORY:
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return HSQS_INODE_TYPE_DIRECTORY;
	case HSQS_INODE_TYPE_BASIC_FILE:
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return HSQS_INODE_TYPE_FILE;
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return HSQS_INODE_TYPE_SYMLINK;
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
		return HSQS_INODE_TYPE_BLOCK;
	case HSQS_INODE_TYPE_BASIC_CHAR:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return HSQS_INODE_TYPE_CHAR;
	case HSQS_INODE_TYPE_BASIC_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
		return HSQS_INODE_TYPE_FIFO;
	case HSQS_INODE_TYPE_BASIC_SOCKET:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return HSQS_INODE_TYPE_SOCKET;
	}
	return HSQS_INODE_TYPE_UNKNOWN;
}

const char *
hsqs_inode_symlink(const struct HsqsInodeContext *context) {
	const struct HsqsInodeSymlink *basic;
	const struct HsqsInodeSymlinkExt *extended;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		basic = hsqs_data_inode_symlink(inode);
		return (const char *)hsqs_data_inode_symlink_target_path(basic);
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		extended = hsqs_data_inode_symlink_ext(inode);
		return (const char *)hsqs_data_inode_symlink_ext_target_path(extended);
	}
	return NULL;
}

int
hsqs_inode_symlink_dup(
		const struct HsqsInodeContext *inode, char **namebuffer) {
	int size = hsqs_inode_symlink_size(inode);
	const char *link_target = hsqs_inode_symlink(inode);

	*namebuffer = hsqs_memdup(link_target, size);
	if (*namebuffer) {
		return size;
	} else {
		return -HSQS_ERROR_MALLOC_FAILED;
	}
}

uint32_t
hsqs_inode_symlink_size(const struct HsqsInodeContext *context) {
	const struct HsqsInodeSymlink *basic;
	const struct HsqsInodeSymlinkExt *extended;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_SYMLINK:
		basic = hsqs_data_inode_symlink(inode);
		return hsqs_data_inode_symlink_target_size(basic);
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		extended = hsqs_data_inode_symlink_ext(inode);
		return hsqs_data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

uint32_t
hsqs_inode_device_id(const struct HsqsInodeContext *context) {
	const struct HsqsInodeDevice *basic;
	const struct HsqsInodeDeviceExt *extended;

	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_BASIC_BLOCK:
	case HSQS_INODE_TYPE_BASIC_CHAR:
		basic = hsqs_data_inode_device(inode);
		return hsqs_data_inode_device_device(basic);
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		extended = hsqs_data_inode_device_ext(inode);
		return hsqs_data_inode_device_ext_device(extended);
	}
	return 0;
}

static uint32_t
inode_get_id(const struct HsqsInodeContext *context, off_t idx) {
	int rv = 0;
	struct HsqsTable *id_table;
	uint32_t id;

	rv = hsqs_id_table(context->hsqs, &id_table);
	if (rv < 0) {
		return UINT32_MAX;
	}

	rv = hsqs_table_get(id_table, idx, &id);
	if (rv < 0) {
		return UINT32_MAX;
	}
	return id;
}

uint32_t
hsqs_inode_uid(const struct HsqsInodeContext *context) {
	return inode_get_id(context, hsqs_data_inode_uid_idx(get_inode(context)));
}

uint32_t
hsqs_inode_gid(const struct HsqsInodeContext *context) {
	return inode_get_id(context, hsqs_data_inode_gid_idx(get_inode(context)));
}

uint32_t
hsqs_inode_xattr_index(const struct HsqsInodeContext *context) {
	const struct HsqsInode *inode = get_inode(context);
	switch (hsqs_data_inode_type(inode)) {
	case HSQS_INODE_TYPE_EXTENDED_DIRECTORY:
		return hsqs_data_inode_directory_ext_xattr_idx(
				hsqs_data_inode_directory_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_FILE:
		return hsqs_data_inode_file_ext_xattr_idx(
				hsqs_data_inode_file_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_SYMLINK:
		return hsqs_data_inode_symlink_ext_xattr_idx(
				hsqs_data_inode_symlink_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_BLOCK:
	case HSQS_INODE_TYPE_EXTENDED_CHAR:
		return hsqs_data_inode_device_ext_xattr_idx(
				hsqs_data_inode_device_ext(inode));
	case HSQS_INODE_TYPE_EXTENDED_FIFO:
	case HSQS_INODE_TYPE_EXTENDED_SOCKET:
		return hsqs_data_inode_ipc_ext_xattr_idx(
				hsqs_data_inode_ipc_ext(inode));
	}
	return HSQS_INODE_NO_XATTR;
}

int
hsqs_inode_xattr_iterator(
		const struct HsqsInodeContext *inode,
		struct HsqsXattrIterator *iterator) {
	int rv = 0;
	struct HsqsXattrTable *table = NULL;

	rv = hsqs_xattr_table(inode->hsqs, &table);
	if (rv < 0 && rv != -HSQS_ERROR_NO_XATTR_TABLE) {
		return rv;
	}
	return hsqs_xattr_iterator_init(iterator, table, inode);
}

int
hsqs_inode_cleanup(struct HsqsInodeContext *inode) {
	return hsqs_metablock_stream_cleanup(&inode->metablock);
}

void
hsqs_inode_ref_to_block(uint64_t ref, uint32_t *block_index, uint16_t *offset) {
	*block_index = (ref & 0x0000FFFFFFFF0000) >> 16;
	*offset = ref & 0x000000000000FFFF;
}
uint64_t
hsqs_inode_ref_from_block(uint32_t block_index, uint16_t offset) {
	return ((uint64_t)block_index << 16) | offset;
}
