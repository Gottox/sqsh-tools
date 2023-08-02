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
 * @author       Enno Boland (mail@eboland.de)
 * @file         fs.c
 */

#include "fs-common.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SQSHFS_OPT(t, p, v) \
	{ t, offsetof(struct SqshfsOptions, p), v }
struct fuse_opt fs_common_opts[] = {
		SQSHFS_OPT("archive=%s", archive, 0),
		SQSHFS_OPT("offset=%s", offset, 0),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_END,
};

void
fs_common_help(void) {
	printf("    -o archive=PATH        squashfs archive to be mounted\n");
	printf("    -o offset=OFFSET       skip OFFSET at the start of archive\n");
}

void
fs_common_usage(const char *progname) {
	fprintf(stderr,
			"usage: %s ARCHIVE MOUNTPOINT [OPTIONS]\n"
			"       %s MOUNTPOINT -o source=ARCHIVE [OPTIONS]\n",
			progname, progname);
}

void
fs_common_version(const char *progname) {
	fputs(progname, stderr);
	fputs("-" VERSION "\n", stderr);
}

mode_t
fs_common_mode_type(enum SqshInodeType type) {
	switch (type) {
	case SQSH_INODE_TYPE_DIRECTORY:
		return S_IFDIR;
	case SQSH_INODE_TYPE_FILE:
		return S_IFREG;
	case SQSH_INODE_TYPE_SYMLINK:
		return S_IFLNK;
	case SQSH_INODE_TYPE_BLOCK:
		return S_IFBLK;
	case SQSH_INODE_TYPE_CHAR:
		return S_IFCHR;
	case SQSH_INODE_TYPE_FIFO:
		return S_IFIFO;
	case SQSH_INODE_TYPE_SOCKET:
		return S_IFSOCK;
	case SQSH_INODE_TYPE_UNKNOWN:
		return 0;
	}
	return 0;
}

// fuse reserves 1 as the root inode. In order to avoid collisions with the
// root inode, we need to map between the fuse inode and the sqsh inode by
// setting them off by one.
uint_fast64_t
fs_common_inode_sqsh_to_ino(uint_fast64_t inode) {
	return inode + 1;
}

uint_fast64_t
fs_common_inode_sqsh_from_ino(uint_fast64_t st_ino) {
	return st_ino - 1;
}

mode_t
fs_common_inode_mode(struct SqshInode *inode) {
	mode_t mode = sqsh_inode_permission(inode);
	return mode | fs_common_mode_type(sqsh_inode_type(inode));
}

int
fs_common_map_err(int rv) {
	if (rv >= 0) {
		return rv;
	}

	enum SqshError err = -rv;
	switch (err) {
	case SQSH_ERROR_NO_SUCH_FILE:
		return -ENOENT;
		break;
	case SQSH_ERROR_NOT_A_DIRECTORY:
		return -ENOTDIR;
	default:
		return -EIO;
	}
}

int
fs_common_read(
		struct SqshFileReader **reader, struct SqshInode *inode, off_t offset,
		size_t size) {
	int rv = 0;
	*reader = sqsh_file_reader_new(inode, &rv);
	if (rv < 0) {
		goto out;
	}

	uint64_t file_size = sqsh_inode_file_size(inode);
	if (size > file_size - offset) {
		size = file_size - offset;
	}

	rv = sqsh_file_reader_advance(*reader, offset, size);
	if (rv < 0) {
		goto out;
	}
out:

	return fs_common_map_err(rv);
}

void
fs_common_getattr(
		struct SqshInode *inode, const struct SqshSuperblock *superblock,
		struct stat *st) {
	const uint64_t inode_number = sqsh_inode_number(inode);

	st->st_dev = 0;
	st->st_ino = fs_common_inode_sqsh_to_ino(inode_number);
	st->st_mode = fs_common_inode_mode(inode);
	st->st_nlink = sqsh_inode_hard_link_count(inode);
	st->st_uid = sqsh_inode_uid(inode);
	st->st_gid = sqsh_inode_gid(inode);
	st->st_size = sqsh_inode_file_size(inode);
	st->st_atime = st->st_mtime = st->st_ctime =
			sqsh_inode_modified_time(inode);
	if (superblock != NULL) {
		st->st_blksize = sqsh_superblock_block_size(superblock);
	}
}
