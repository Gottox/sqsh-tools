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
 * @file         sqsh_archive_builder.h
 */

#ifndef SQSH_INODE_BUILDER_H
#define SQSH_INODE_BUILDER_H

#include "../../../include/sqsh_data.h"
#include "../../../include/sqsh_data_set.h"
#include <cextras/collection.h>

#include <stdio.h>

struct SqshMetablockBuilder;

/***************************************
 * archive/inode_builder.c
 */
struct SqshInodeBuilder {
	uint32_t inode_count;
	struct SqshDataInode data;
	enum SqshDataInodeType type;
	uint32_t xattr_index;
	struct CxBuffer post_data;
};

int sqsh__inode_builder_init(struct SqshInodeBuilder *inode);

int sqsh__inode_builder_permission(
		struct SqshInodeBuilder *inode, uint16_t permission);

int sqsh__inode_builder_uid_idx(struct SqshInodeBuilder *inode, uint16_t uid);

int sqsh__inode_builder_gid_idx(struct SqshInodeBuilder *inode, uint16_t gid);

int sqsh__inode_builder_modification_time(
		struct SqshInodeBuilder *inode, uint32_t mtime);

int sqsh__inode_builder_inode_number(
		struct SqshInodeBuilder *inode, uint32_t inode_number);

int sqsh__inode_builder_symlink(
		struct SqshInodeBuilder *inode, const char *symlink);

int
sqsh__inode_builder_xattr_index(struct SqshInodeBuilder *inode, uint32_t index);

int sqsh__inode_builder_extend(struct SqshInodeBuilder *inode);

int sqsh__inode_builder_write(
		struct SqshInodeBuilder *inode, struct SqshMetablockBuilder *output);

int sqsh__inode_builder_cleanup(struct SqshInodeBuilder *inode);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_INODE_BUILDER_H */
