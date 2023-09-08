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
 * @file         inode_builder.c
 */

#define _DEFAULT_SOURCE

#include "../../../include/sqsh_data.h"
#include "../../../include/sqsh_data_set.h"
#include "../../../include/sqsh_error.h"
#include "../../read/utils/utils.h"
#include "../sqsh_file_builder.h"
#include "../sqsh_metablock_builder.h"
#include <string.h>

int
sqsh__inode_builder_init(struct SqshInodeBuilder *inode) {
	memset(inode, 0, sizeof(*inode));
	return 0;
}

int
sqsh__inode_builder_permission(
		struct SqshInodeBuilder *inode, uint16_t permission) {
	sqsh__data_inode_permissions_set(&inode->data, permission);
	return 0;
}

int
sqsh__inode_builder_uid_idx(struct SqshInodeBuilder *inode, uint16_t uid) {
	sqsh__data_inode_uid_idx_set(&inode->data, uid);
	return 0;
}

int
sqsh__inode_builder_gid_idx(struct SqshInodeBuilder *inode, uint16_t gid) {
	sqsh__data_inode_gid_idx_set(&inode->data, gid);
	return 0;
}

int
sqsh__inode_builder_modification_time(
		struct SqshInodeBuilder *inode, uint32_t mtime) {
	sqsh__data_inode_modified_time_set(&inode->data, mtime);
	return 0;
}

int
sqsh__inode_builder_inode_number(
		struct SqshInodeBuilder *inode, uint32_t inode_number) {
	sqsh__data_inode_number_set(&inode->data, inode_number);
	return 0;
}

int
sqsh__inode_builder_symlink(
		struct SqshInodeBuilder *inode, const char *symlink) {
	inode->type = SQSH_INODE_TYPE_BASIC_SYMLINK;
	return cx_buffer_append(
			&inode->post_data, (const uint8_t *)symlink, strlen(symlink));
	return 0;
}

int
sqsh__inode_builder_xattr_index(
		struct SqshInodeBuilder *inode, uint32_t index) {
	inode->xattr_index = index;
	return 0;
}

int
sqsh__inode_builder_extend(struct SqshInodeBuilder *inode) {
	switch (inode->type) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		inode->type = SQSH_INODE_TYPE_EXTENDED_DIRECTORY;
		break;
	case SQSH_INODE_TYPE_BASIC_FILE:
		inode->type = SQSH_INODE_TYPE_EXTENDED_FILE;
		break;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		inode->type = SQSH_INODE_TYPE_EXTENDED_SYMLINK;
		break;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
		inode->type = SQSH_INODE_TYPE_EXTENDED_BLOCK;
		break;
	case SQSH_INODE_TYPE_BASIC_CHAR:
		inode->type = SQSH_INODE_TYPE_EXTENDED_CHAR;
		break;
	case SQSH_INODE_TYPE_BASIC_FIFO:
		inode->type = SQSH_INODE_TYPE_EXTENDED_FIFO;
		break;
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		inode->type = SQSH_INODE_TYPE_EXTENDED_SOCKET;
		break;
	default:
		break;
	}
	return 0;
}

static size_t
prepare_symlink(struct SqshInodeBuilder *inode) {
	struct SqshDataInodeSymlink *data =
			sqsh__data_inode_symlink_mut(&inode->data);
	const size_t post_data_size = cx_buffer_size(&inode->post_data);

	sqsh__data_inode_symlink_hard_link_count_set(data, inode->inode_count);
	sqsh__data_inode_symlink_target_size_set(data, post_data_size);

	return sizeof(struct SqshDataInodeHeader) +
			sizeof(struct SqshDataInodeSymlink);
}

static size_t
prepare_symlink_ext(struct SqshInodeBuilder *inode) {
	struct SqshDataInodeSymlinkExt *data =
			sqsh__data_inode_symlink_ext_mut(&inode->data);
	const size_t post_data_size = cx_buffer_size(&inode->post_data);

	sqsh__data_inode_symlink_ext_hard_link_count_set(data, inode->inode_count);
	sqsh__data_inode_symlink_ext_target_size_set(data, post_data_size);

	return sizeof(struct SqshDataInodeHeader) +
			sizeof(struct SqshDataInodeSymlinkExt);
}

int
sqsh__inode_builder_write(
		struct SqshInodeBuilder *inode, struct SqshMetablockBuilder *output) {
	int rv = 0;
	size_t data_size = 0;
	sqsh__data_inode_type_set(&inode->data, inode->type);

	switch (inode->type) {
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		data_size = prepare_symlink(inode);
		break;
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		data_size = prepare_symlink_ext(inode);
		break;
	default:
		// TODO: Not implemented
		abort();
	}

	const uint8_t *data = (uint8_t *)&inode->data;
	rv = sqsh__metablock_builder_write(output, data, data_size);
	if (rv < 0) {
		goto out;
	}
	data = cx_buffer_data(&inode->post_data);
	data_size = cx_buffer_size(&inode->post_data);
	rv = sqsh__metablock_builder_write(output, data, data_size);
	if (rv < 0) {
		goto out;
	}

	// The extended symlink is unique in that it has an additional field
	// after the post data.
	if (inode->type == SQSH_INODE_TYPE_EXTENDED_SYMLINK) {
		struct SqshDataInodeSymlinkExtTail tail = {0};
		sqsh__data_inode_symlink_ext_tail_xattr_idx_set(
				&tail, inode->xattr_index);

		data = (uint8_t *)&tail;
		data_size = sizeof(tail);
		rv = sqsh__metablock_builder_write(output, data, data_size);
		if (rv < 0) {
			goto out;
		}
	}

	rv = data_size + cx_buffer_size(&inode->post_data);
out:
	return rv;
}

int
sqsh__inode_builder_cleanup(struct SqshInodeBuilder *inode) {
	cx_buffer_cleanup(&inode->post_data);
	return 0;
}
