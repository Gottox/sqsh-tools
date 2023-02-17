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
 * @file         sqsh-mount.c
 */

#include "common.h"

#include "../src/utils.h"
#include <sqsh_context.h>
#include <sqsh_directory.h>
#include <sqsh_xattr.h>

#define FUSE_USE_VERSION 35
#include <errno.h>
#include <fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static struct {
	struct Sqsh *sqsh;
	struct SqshPathResolverContext *resolver;
} data = {0};

static struct SqshfuseOptions {
	int show_help;
	const char *image_path;
} options = {0};

#define SQSH_OPT_KEY(t, p) \
	{ t, offsetof(struct SqshfuseOptions, p), 1 }
// clang-format off
static const struct fuse_opt option_spec[] = {
	SQSH_OPT_KEY("-h", show_help),
	SQSH_OPT_KEY("--help", show_help),
	FUSE_OPT_END
};
// clang-format on

static void
help(const char *arg0) {
	(void)arg0;
	// TODO
}

static void *
sqshfuse_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
	(void)conn;
	(void)cfg;
	int rv = 0;
	struct fuse_context *context = fuse_get_context();

	data.sqsh = open_archive(options.image_path, &rv);
	if (rv < 0) {
		sqsh_perror(rv, options.image_path);
		fuse_unmount(context->fuse);
		exit(EXIT_FAILURE);
	}
	data.resolver = sqsh_path_resolver_new(data.sqsh, &rv);
	if (rv < 0) {
		sqsh_perror(rv, options.image_path);
		fuse_unmount(context->fuse);
		exit(EXIT_FAILURE);
	}

	return NULL;
}

static int
sqshfuse_getattr(
		const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	(void)fi;
	int rv = 0;
	memset(stbuf, 0, sizeof(struct stat));

	struct SqshInodeContext *inode = NULL;
	struct SqshSuperblockContext *superblock = sqsh_superblock(data.sqsh);

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	stbuf->st_ino = sqsh_inode_number(inode);
	stbuf->st_mode = sqsh_inode_permission(inode);
	stbuf->st_nlink = sqsh_inode_hard_link_count(inode);
	stbuf->st_uid = sqsh_inode_uid(inode);
	stbuf->st_gid = sqsh_inode_gid(inode);
	stbuf->st_rdev = sqsh_inode_device_id(inode);
	stbuf->st_size = sqsh_inode_file_size(inode);
	stbuf->st_blksize = sqsh_superblock_block_size(superblock);
	stbuf->st_mtime = stbuf->st_ctime = stbuf->st_atime =
			sqsh_inode_modified_time(inode);
	switch (sqsh_inode_type(inode)) {
	case SQSH_INODE_TYPE_DIRECTORY:
		stbuf->st_mode |= S_IFDIR;
		break;
	case SQSH_INODE_TYPE_FILE:
		stbuf->st_mode |= S_IFREG;
		break;
	case SQSH_INODE_TYPE_SYMLINK:
		stbuf->st_mode |= S_IFLNK;
		break;
	case SQSH_INODE_TYPE_BLOCK:
		stbuf->st_mode |= S_IFBLK;
		break;
	case SQSH_INODE_TYPE_CHAR:
		stbuf->st_mode |= S_IFCHR;
		break;
	case SQSH_INODE_TYPE_FIFO:
		stbuf->st_mode |= S_IFIFO;
		break;
	case SQSH_INODE_TYPE_SOCKET:
		stbuf->st_mode |= S_IFSOCK;
		break;
	case SQSH_INODE_TYPE_UNKNOWN:
		rv = -EIO;
		goto out;
	}
out:
	sqsh_inode_free(inode);
	return rv;
}

static int
sqshfuse_getxattr(
		const char *path, const char *name, char *value, size_t size) {
	int rv = 0;
	const char *value_ptr = NULL;
	size_t value_size;
	struct SqshInodeContext *inode = NULL;
	struct SqshXattrIterator *iter = NULL;

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	iter = sqsh_xattr_iterator_new(inode, &rv);
	if (rv < 0) {
		// TODO: this means that the archive is corrupt, not that it has no
		// xattrs. Handle the error accordingly.
		rv = -ENODATA;
		goto out;
	}

	while (value_ptr == NULL && (rv = sqsh_xattr_iterator_next(iter)) > 0) {
		if (sqsh_xattr_iterator_fullname_cmp(iter, name) == 0) {
			value_ptr = sqsh_xattr_iterator_value(iter);
			value_size = sqsh_xattr_iterator_value_size(iter);
		}
	}
	if (rv < 0) {
		rv = -EINVAL;
		goto out;
	} else if (value_ptr == NULL) {
		rv = -ENODATA;
		goto out;
	} else if (value_size <= size) {
		value_ptr = sqsh_xattr_iterator_value(iter);
		memcpy(value, value_ptr, value_size);
	} else if (size != 0) {
		rv = -ERANGE;
		goto out;
	}

	rv = value_size;
out:
	sqsh_xattr_iterator_free(iter);
	sqsh_inode_free(inode);
	return rv;
}
static int
sqshfuse_listxattr(const char *path, char *list, size_t size) {
	int rv = 0;
	size_t element_length, length;
	const char *prefix, *name;
	char *p;
	struct SqshInodeContext *inode = NULL;
	struct SqshXattrIterator *iter = NULL;

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	iter = sqsh_xattr_iterator_new(inode, &rv);
	if (rv < 0) {
		rv = -EINVAL; // TODO: find correct error code for this.
		goto out;
	}

	p = list;
	length = 0;
	while ((rv = sqsh_xattr_iterator_next(iter)) > 0) {
		prefix = sqsh_xattr_iterator_prefix(iter);
		if (prefix == NULL) {
			rv = -EINVAL; // TODO: find correct error code for this.
			goto out;
		}
		element_length = sqsh_xattr_iterator_prefix_size(iter);
		length += element_length;
		if (length < size) {
			strcpy(p, prefix);
			p = &list[length];
		}

		name = sqsh_xattr_iterator_name(iter);
		if (name == NULL) {
			rv = -EINVAL; // TODO: find correct error code for this.
			goto out;
		}
		element_length = sqsh_xattr_iterator_name_size(iter);
		length += element_length;
		if (length + 1 < size) {
			strcpy(p, name);
			p[element_length] = '\0';
			length++;
			p = &list[length];
		}
	}
	if (rv < 0) {
		rv = -EINVAL;
		goto out;
	}

	rv = length;

out:
	sqsh_xattr_iterator_free(iter);
	sqsh_inode_free(inode);
	return rv;
}

static int
sqshfuse_readdir(
		const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	(void)fi; // TODO
	(void)offset; // TODO
	(void)flags; // TODO
	int rv = 0;
	struct SqshInodeContext *inode = NULL;
	struct SqshDirectoryIterator *iter = NULL;
	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	iter = sqsh_directory_iterator_new(inode, &rv);
	if (rv < 0) {
		rv = -ENOMEM;
		goto out;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);

	while (sqsh_directory_iterator_next(iter) > 0) {
		char *name;
		rv = sqsh_directory_iterator_name_dup(iter, &name);
		if (rv < 0) {
			rv = -ENOMEM;
			goto out;
		}
		rv = filler(buf, name, NULL, 0, 0);
		if (rv < 0) {
			rv = -ENOMEM;
			goto out;
		}
		free(name);
	}

out:
	sqsh_directory_iterator_free(iter);
	sqsh_inode_free(inode);
	return rv;
}

static int
sqshfuse_open(const char *path, struct fuse_file_info *fi) {
	int rv = 0;
	struct SqshInodeContext *inode = NULL;

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		rv = -EACCES;
		goto out;
	}

out:
	sqsh_inode_free(inode);
	return rv;
}

static int
sqshfuse_read(
		const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	(void)fi;
	int rv = 0;
	struct SqshInodeContext *inode = NULL;
	struct SqshFileContext *file = NULL;

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}
	file = sqsh_file_new(inode, &rv);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}

	size = SQSH_MIN(size, sqsh_inode_file_size(inode));
	rv = sqsh_file_seek(file, offset);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}
	rv = sqsh_file_read(file, size);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}

	if (size != 0) {
		memcpy(buf, sqsh_file_data(file), size);
	}

	rv = size;
out:
	sqsh_file_free(file);
	sqsh_inode_free(inode);
	return rv;
}

static int
sqshfuse_readlink(const char *path, char *buf, size_t size) {
	int rv = 0;
	struct SqshInodeContext *inode = NULL;

	inode = sqsh_path_resolver_resolve(data.resolver, path, &rv);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	const char *symlink = sqsh_inode_symlink(inode);
	size_t symlink_size = sqsh_inode_symlink_size(inode);
	size_t cpy_size = SQSH_MIN(symlink_size, size - 1);

	memcpy(buf, symlink, cpy_size);
	buf[cpy_size] = 0;

out:
	sqsh_inode_free(inode);
	return rv;
}

static void
sqshfuse_destroy(void *private_data) {
	(void)private_data;
	sqsh_path_resolver_free(data.resolver);
	sqsh_free(data.sqsh);
}

static const struct fuse_operations sqshfuse_operations = {
		.init = sqshfuse_init,
		.getattr = sqshfuse_getattr,
		.getxattr = sqshfuse_getxattr,
		.listxattr = sqshfuse_listxattr,
		.readdir = sqshfuse_readdir,
		.open = sqshfuse_open,
		.read = sqshfuse_read,
		.readlink = sqshfuse_readlink,
		.destroy = sqshfuse_destroy,
};

static int
sqshfuse_process_options(
		void *data, const char *arg, int key, struct fuse_args *outargs) {
	(void)data;
	(void)outargs;
	if (key == FUSE_OPT_KEY_NONOPT && options.image_path == NULL) {
		options.image_path = arg;
		return 0;
	}
	return 1;
}

int
main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	int rv = 0;
	if (fuse_opt_parse(
				&args, &options, option_spec, sqshfuse_process_options) == -1) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (options.show_help) {
		help(argv[0]);
		fuse_opt_add_arg(&args, "--help");
		args.argv[0][0] = '\0';
	}

	rv = fuse_main(args.argc, args.argv, &sqshfuse_operations, &data);
out:
	fuse_opt_free_args(&args);
	return rv;
}
