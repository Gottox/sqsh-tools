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
 * @file        : hsqs-mount
 * @created     : Wednesday Sep 08, 2021 09:06:25 CEST
 */

#define FUSE_USE_VERSION 35

#include <errno.h>
#include <fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../src/context/content_context.h"
#include "../src/context/inode_context.h"
#include "../src/hsqs.h"
#include "../src/iterator/directory_iterator.h"
#include "../src/iterator/xattr_iterator.h"

static struct { struct Hsqs hsqs; } data = {0};

static struct HsqsfuseOptions {
	int show_help;
	const char *image_path;
} options = {0};

#define HSQS_OPT_KEY(t, p) \
	{ t, offsetof(struct HsqsfuseOptions, p), 1 }
// clang-format off
static const struct fuse_opt option_spec[] = {
	HSQS_OPT_KEY("-h", show_help),
	HSQS_OPT_KEY("--help", show_help),
	FUSE_OPT_END
};
// clang-format on

static void
help(const char *arg0) {
	(void)arg0;
	// TODO
}

static void *
hsqsfuse_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
	(void)conn;
	(void)cfg;
	int rv = 0;
	struct fuse_context *context = fuse_get_context();

	rv = hsqs_open(&data.hsqs, options.image_path);
	if (rv < 0) {
		hsqs_perror(rv, options.image_path);
		fuse_unmount(context->fuse);
		exit(EXIT_FAILURE);
	}

	return NULL;
}

static int
hsqsfuse_getattr(
		const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	(void)fi;
	int rv = 0;
	memset(stbuf, 0, sizeof(struct stat));

	struct HsqsInodeContext inode = {0};
	struct HsqsSuperblockContext *superblock = hsqs_superblock(&data.hsqs);

	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	stbuf->st_ino = hsqs_inode_number(&inode);
	stbuf->st_mode = hsqs_inode_permission(&inode);
	stbuf->st_nlink = hsqs_inode_hard_link_count(&inode);
	stbuf->st_uid = hsqs_inode_uid(&inode);
	stbuf->st_gid = hsqs_inode_gid(&inode);
	stbuf->st_rdev = hsqs_inode_device_id(&inode);
	stbuf->st_size = hsqs_inode_file_size(&inode);
	stbuf->st_blksize = hsqs_superblock_block_size(superblock);
	stbuf->st_mtime = stbuf->st_ctime = stbuf->st_atime =
			hsqs_inode_modified_time(&inode);
	switch (hsqs_inode_type(&inode)) {
	case HSQS_INODE_TYPE_DIRECTORY:
		stbuf->st_mode |= S_IFDIR;
		break;
	case HSQS_INODE_TYPE_FILE:
		stbuf->st_mode |= S_IFREG;
		break;
	case HSQS_INODE_TYPE_SYMLINK:
		stbuf->st_mode |= S_IFLNK;
		break;
	case HSQS_INODE_TYPE_BLOCK:
		stbuf->st_mode |= S_IFBLK;
		break;
	case HSQS_INODE_TYPE_CHAR:
		stbuf->st_mode |= S_IFCHR;
		break;
	case HSQS_INODE_TYPE_FIFO:
		stbuf->st_mode |= S_IFIFO;
		break;
	case HSQS_INODE_TYPE_SOCKET:
		stbuf->st_mode |= S_IFSOCK;
		break;
	case HSQS_INODE_TYPE_UNKNOWN:
		rv = -EIO;
		goto out;
	}
out:
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
hsqsfuse_getxattr(
		const char *path, const char *name, char *value, size_t size) {
	int rv = 0;
	char *fullname;
	const char *value_ptr = NULL;
	size_t value_size;
	struct HsqsInodeContext inode = {0};
	struct HsqsXattrIterator iter = {0};

	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	rv = hsqs_inode_xattr_iterator(&inode, &iter);
	if (rv < 0) {
		rv = -EINVAL; // TODO: find correct error code for this.
		goto out;
	}

	while (value_ptr == NULL && (rv = hsqs_xattr_iterator_next(&iter)) > 0) {
		rv = hsqs_xattr_iterator_fullname_dup(&iter, &fullname);
		if (rv < 0) {
			rv = -EINVAL; // TODO: find correct error code for this.
			goto out;
		}

		if (strcmp(fullname, name) == 0) {
			value_ptr = hsqs_xattr_iterator_value(&iter);
			value_size = hsqs_xattr_iterator_value_size(&iter);
		}
		free(fullname);
	}
	if (rv < 0) {
		rv = -EINVAL;
		goto out;
	} else if (value_ptr == NULL) {
		rv = -ENODATA;
		goto out;
	} else if (value_size <= size) {
		value_ptr = hsqs_xattr_iterator_value(&iter);
		memcpy(value, value_ptr, value_size);
	} else if (size != 0) {
		rv = -ERANGE;
		goto out;
	}

	rv = value_size;
out:
	hsqs_xattr_iterator_cleanup(&iter);
	hsqs_inode_cleanup(&inode);
	return rv;
}
static int
hsqsfuse_listxattr(const char *path, char *list, size_t size) {
	int rv = 0;
	size_t element_length, length;
	const char *prefix, *name;
	char *p;
	struct HsqsInodeContext inode = {0};
	struct HsqsXattrIterator iter = {0};
	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	rv = hsqs_inode_xattr_iterator(&inode, &iter);
	if (rv < 0) {
		rv = -EINVAL; // TODO: find correct error code for this.
		goto out;
	}

	p = list;
	length = 0;
	while ((rv = hsqs_xattr_iterator_next(&iter)) > 0) {
		prefix = hsqs_xattr_iterator_prefix(&iter);
		if (prefix == NULL) {
			rv = -EINVAL; // TODO: find correct error code for this.
			goto out;
		}
		element_length = hsqs_xattr_iterator_prefix_size(&iter);
		length += element_length;
		if (length < size) {
			strcpy(p, prefix);
			p = &list[length];
		}

		name = hsqs_xattr_iterator_name(&iter);
		if (name == NULL) {
			rv = -EINVAL; // TODO: find correct error code for this.
			goto out;
		}
		element_length = hsqs_xattr_iterator_name_size(&iter);
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
	hsqs_xattr_iterator_cleanup(&iter);
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
hsqsfuse_readdir(
		const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	(void)fi; // TODO
	(void)offset; // TODO
	(void)flags; // TODO
	int rv = 0;
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryIterator iter = {0};
	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	rv = hsqs_directory_iterator_init(&iter, &inode);
	if (rv < 0) {
		rv = -ENOMEM;
		goto out;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);

	while (hsqs_directory_iterator_next(&iter) > 0) {
		char *name;
		rv = hsqs_directory_iterator_name_dup(&iter, &name);
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
	hsqs_directory_iterator_cleanup(&iter);
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
hsqsfuse_open(const char *path, struct fuse_file_info *fi) {
	int rv = 0;
	struct HsqsInodeContext inode = {0};

	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		rv = -EACCES;
		goto out;
	}

out:
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
hsqsfuse_read(
		const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	(void)fi;
	int rv = 0;
	struct HsqsInodeContext inode = {0};
	struct HsqsFileContext file = {0};

	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}
	rv = hsqs_content_init(&file, &inode);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}

	size = MIN(size, hsqs_inode_file_size(&inode));
	rv = hsqs_content_seek(&file, offset);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}
	rv = hsqs_content_read(&file, size);
	if (rv < 0) {
		// TODO: Better return type
		rv = -EINVAL;
		goto out;
	}

	memcpy(buf, hsqs_content_data(&file), size);

	rv = size;
out:
	hsqs_content_cleanup(&file);
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
hsqsfuse_readlink(const char *path, char *buf, size_t size) {
	int rv = 0;
	struct HsqsInodeContext inode = {0};

	rv = hsqs_inode_load_by_path(&inode, &data.hsqs, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	const char *symlink = hsqs_inode_symlink(&inode);
	size_t symlink_size = hsqs_inode_symlink_size(&inode);
	size_t cpy_size = MIN(symlink_size, size - 1);

	memcpy(buf, symlink, cpy_size);
	buf[cpy_size] = 0;

out:
	hsqs_inode_cleanup(&inode);
	return rv;
}

static const struct fuse_operations hsqsfuse_operations = {
		.init = hsqsfuse_init,
		.getattr = hsqsfuse_getattr,
		.getxattr = hsqsfuse_getxattr,
		.listxattr = hsqsfuse_listxattr,
		.readdir = hsqsfuse_readdir,
		.open = hsqsfuse_open,
		.read = hsqsfuse_read,
		.readlink = hsqsfuse_readlink,
};

static int
hsqsfuse_process_options(
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
				&args, &options, option_spec, hsqsfuse_process_options) == -1) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (options.show_help) {
		help(argv[0]);
		fuse_opt_add_arg(&args, "--help");
		args.argv[0][0] = '\0';
	}

	rv = fuse_main(args.argc, args.argv, &hsqsfuse_operations, &data);
out:
	fuse_opt_free_args(&args);
	return rv;
}
