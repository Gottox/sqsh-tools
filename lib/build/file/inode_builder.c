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

#include "../../../include/sqsh_common.h"
#include "../../../include/sqsh_data_private.h"
#include "../../../include/sqsh_data_set.h"
#include "../../../include/sqsh_error.h"
#include "../sqsh_file_builder.h"
#include <stdlib.h>
#include <string.h>

#define XATTR_UNSET 0xFFFFFFFF

int
sqsh__inode_builder_init(struct SqshInodeBuilder *builder) {
	memset(builder, 0, sizeof(*builder));
	cx_rc_init(&builder->rc);
	return 0;
}

int
sqsh__inode_builder_cleanup(struct SqshInodeBuilder *builder) {
	int rv = 0;
	if (builder->payload != NULL) {
		rv = builder->payload->free(builder->payload_data);
		if (rv < 0) {
			goto out;
		}
		builder->payload = NULL;
		builder->payload_data = NULL;
	}
out:
	return rv;
}

struct SqshInodeBuilder *
sqsh__inode_builder_new(int *err) {
	int rv = 0;
	struct SqshInodeBuilder *builder =
			calloc(1, sizeof(struct SqshInodeBuilder));
	if (builder == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = sqsh__inode_builder_init(builder);
	if (rv < 0) {
		free(builder);
		builder = NULL;
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	return builder;
}

int
sqsh__inode_builder_type(
		struct SqshInodeBuilder *inode, enum SqshDataInodeType type) {
	inode->type = type;
	sqsh__data_inode_type_set(&inode->inode, type);
	return 0;
}

int
sqsh__inode_builder_permission(
		struct SqshInodeBuilder *inode, uint16_t permission) {
	sqsh__data_inode_permissions_set(&inode->inode, permission);
	return 0;
}

int
sqsh__inode_builder_uid(struct SqshInodeBuilder *inode, uint32_t uid) {
	// TODO
	inode->uid = uid;
	return 0;
}

int
sqsh__inode_builder_gid(struct SqshInodeBuilder *inode, uint32_t gid) {
	// TODO
	inode->gid = gid;
	return 0;
}

int
sqsh__inode_builder_modification_time(
		struct SqshInodeBuilder *inode, uint32_t modification_time) {
	sqsh__data_inode_modified_time_set(&inode->inode, modification_time);
	return 0;
}

int
sqsh__inode_builder_number(
		struct SqshInodeBuilder *inode, uint32_t inode_number) {
	inode->inode_number = inode_number;
	sqsh__data_inode_number_set(&inode->inode, inode_number);
	return 0;
}

struct SqshDataInodeDirectory *
sqsh__inode_builder_directory(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_directory_mut(&inode->inode);
}

struct SqshDataInodeFile *
sqsh__inode_builder_file(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_file_mut(&inode->inode);
}

struct SqshDataInodeSymlink *
sqsh__inode_builder_symlink(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_symlink_mut(&inode->inode);
}

struct SqshDataInodeDevice *
sqsh__inode_builder_device(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_device_mut(&inode->inode);
}

struct SqshDataInodeIpc *
sqsh__inode_builder_ipc(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_ipc_mut(&inode->inode);
}

struct SqshDataInodeDirectoryExt *
sqsh__inode_builder_directory_ext(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_directory_ext_mut(&inode->inode);
}

struct SqshDataInodeFileExt *
sqsh__inode_builder_file_ext(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_file_ext_mut(&inode->inode);
}

struct SqshDataInodeSymlinkExt *
sqsh__inode_builder_symlink_ext(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_symlink_ext_mut(&inode->inode);
}

struct SqshDataInodeDeviceExt *
sqsh__inode_builder_device_ext(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_device_ext_mut(&inode->inode);
}

struct SqshDataInodeIpcExt *
sqsh__inode_builder_ipc_ext(struct SqshInodeBuilder *inode) {
	return sqsh__data_inode_ipc_ext_mut(&inode->inode);
}

int
sqsh__inode_builder_payload(
		struct SqshInodeBuilder *inode,
		const struct SqshInodeBuilderPayloadImpl *payload, void *data) {
	inode->payload = payload;
	inode->payload_data = data;

	return 0;
}

int
sqsh__inode_builder_write(
		struct SqshInodeBuilder *builder,
		struct SqshMetablockBuilder *metablock) {
	int rv = 0;
	struct SqshDataInode *inode = &builder->inode;
	size_t size;
	switch (builder->type) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		size = sizeof(struct SqshDataInodeDirectory);
		break;
	case SQSH_INODE_TYPE_BASIC_FILE:
		size = sizeof(struct SqshDataInodeFile);
		break;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		size = sizeof(struct SqshDataInodeSymlink);
		break;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
	case SQSH_INODE_TYPE_BASIC_CHAR:
		size = sizeof(struct SqshDataInodeDevice);
		break;
	case SQSH_INODE_TYPE_BASIC_FIFO:
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		size = sizeof(struct SqshDataInodeIpc);
		break;
	case SQSH_INODE_TYPE_EXTENDED_DIRECTORY:
		size = sizeof(struct SqshDataInodeDirectoryExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_FILE:
		size = sizeof(struct SqshDataInodeFileExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_SYMLINK:
		size = sizeof(struct SqshDataInodeSymlinkExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_BLOCK:
	case SQSH_INODE_TYPE_EXTENDED_CHAR:
		size = sizeof(struct SqshDataInodeDeviceExt);
		break;
	case SQSH_INODE_TYPE_EXTENDED_FIFO:
	case SQSH_INODE_TYPE_EXTENDED_SOCKET:
		size = sizeof(struct SqshDataInodeIpcExt);
		break;
	}
	size += sizeof(struct SqshDataInodeHeader);

	rv = sqsh__metablock_builder_write(metablock, (uint8_t *)inode, size);
	if (rv < 0) {
		goto out;
	}

	if (builder->payload != NULL) {
		rv = builder->payload->write(metablock, builder->payload_data);
		if (rv < 0) {
			goto out;
		}
	}

out:
	return rv;
}

int
inode_builder_free(struct SqshInodeBuilder *builder) {
	if (builder == NULL) {
		return 0;
	}
	int rv = sqsh__inode_builder_cleanup(builder);
	free(builder);
	return rv;
}

int
sqsh__inode_builder_release(struct SqshInodeBuilder *inode) {
	if (inode == NULL) {
		return 0;
	}
	if (cx_rc_release(&inode->rc)) {
		return inode_builder_free(inode);
	}
	return 0;
}
