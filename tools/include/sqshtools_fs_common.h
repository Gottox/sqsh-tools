/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @author   Enno Boland (mail@eboland.de)
 * @file     sqshtools_fs_common.h
 * @created  Wednesday Jun 07, 2023 13:33:20 CEST
 */

#ifndef SQSHTOOLS_FS_COMMON_H

#define SQSHTOOLS_FS_COMMON_H

#include "sqshtools_common.h"

#include "../include/sqsh.h"
#include <fuse_opt.h>
#include <stdint.h>
#include <sys/stat.h>

enum {
	KEY_HELP,
	KEY_VERSION,
};

struct SqshfsOptions {
	const char *archive;
	char *mountpoint;
	int multithreaded;
	int foreground;
	int offset;
};

extern struct fuse_opt fs_common_opts[];

void fs_common_help(void);

void fs_common_usage(const char *progname);

void fs_common_version(const char *progname);

uint_fast64_t fs_common_inode_sqsh_to_ino(uint_fast64_t inode);

uint_fast64_t fs_common_inode_sqsh_from_ino(uint_fast64_t st_ino);

mode_t fs_common_mode_type(enum SqshFileType type);

mode_t fs_common_inode_mode(struct SqshFile *file);

int fs_common_map_err(int rv);

int fs_common_read(
		struct SqshFileReader **reader, struct SqshFile *file, off_t offset,
		size_t size);

void fs_common_getattr(
		struct SqshFile *file, const struct SqshSuperblock *superblock,
		struct stat *st);

#endif /* SQSHTOOLS_FS_COMMON_H */
