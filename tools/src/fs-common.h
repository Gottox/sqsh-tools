/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fs-common
 * @created     : Wednesday Jun 07, 2023 13:33:20 CEST
 */

#ifndef TOOLS_FS_COMMON_H

#define TOOLS_FS_COMMON_H

#include "common.h"

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

#endif // TOOLS_COMMON_H
