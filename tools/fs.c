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

#define FUSE_USE_VERSION 35

#include "common.h"

#include <sqsh_archive.h>
#include <sqsh_directory.h>
#include <sqsh_error.h>
#include <sqsh_file.h>
#include <sqsh_inode.h>
#include <sqsh_table.h>
#include <sqsh_xattr.h>

#include <errno.h>
#include <fuse_lowlevel.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define dbg(...) \
	if (context->debug) { \
		fuse_log(FUSE_LOG_DEBUG, __VA_ARGS__); \
	}

struct Sqshfs {
	pthread_mutex_t lock;
	struct SqshArchive *archive;
	bool debug;
	atomic_uint_fast64_t *inode_map;
};

struct SqshfsOptions {
	const char *archive;
};

struct SqshfsFileHandle {
	struct SqshInode *inode;
};

struct SqshfsDirHandle {
	struct SqshInode *inode;
	struct SqshDirectoryIterator *iterator;
};

static const struct fuse_opt sqshfs_opts[] = {
		{"archive=%s", offsetof(struct SqshfsOptions, archive), 0},
		FUSE_OPT_END};

static void
help(void) {
	printf("    -o archive=PATH        squashfs archive to be mounted\n");
}

static void
usage(const char *progname) {
	fprintf(stderr,
			"usage: %s ARCHIVE MOUNTPOINT [OPTIONS]\n"
			"       %s MOUNTPOINT -o source=ARCHIVE [OPTIONS]\n",
			progname, progname);
}

// fuse reserves 1 as the root inode. In order to avoid collisions with the
// root inode, we need to map between the fuse inode and the sqsh inode by
// setting them off by one.
static fuse_ino_t
inode_sqsh_to_fuse(uint32_t inode) {
	return inode + 1;
}

static uint32_t
inode_fuse_to_sqsh(fuse_ino_t inode) {
	return inode - 1;
}

static uint64_t
sqshfs_context_inode_ref(struct Sqshfs *context, fuse_ino_t fuse_inode) {
	int rv = 0;
	uint64_t inode_ref = 0;
	uint64_t sqsh_inode = inode_fuse_to_sqsh(fuse_inode);
	struct SqshArchive *archive = context->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

	if (fuse_inode == FUSE_ROOT_ID) {
		return sqsh_superblock_inode_root_ref(superblock);
	} else if (context->inode_map[sqsh_inode - 1] != 0) {
		return context->inode_map[sqsh_inode - 1];
	}

	if (sqsh_superblock_has_export_table(superblock) == false) {
		return 0;
	}
	struct SqshExportTable *export_table;
	rv = sqsh_archive_export_table(archive, &export_table);
	if (rv < 0) {
		return 0;
	}

	rv = sqsh_export_table_resolve_inode(export_table, sqsh_inode, &inode_ref);
	if (rv < 0) {
		return 0;
	}

	return inode_ref;
}

static void
sqshfs_init(void *userdata, struct fuse_conn_info *conn) {
	struct Sqshfs *context = userdata;
	dbg("sqshfs_init\n");

	if (conn->capable & FUSE_CAP_PARALLEL_DIROPS) {
		conn->want = FUSE_CAP_PARALLEL_DIROPS;
	}
}

static void
sqshfs_destroy(void *userdata) {
	struct Sqshfs *context = userdata;
	dbg("sqshfs_destroy\n");
}

static mode_t
sqshfs_inode_mode(struct SqshInode *inode) {
	mode_t mode = sqsh_inode_permission(inode);
	switch (sqsh_inode_type(inode)) {
	case SQSH_INODE_TYPE_DIRECTORY:
		mode |= S_IFDIR;
		break;
	case SQSH_INODE_TYPE_FILE:
		mode |= S_IFREG;
		break;
	case SQSH_INODE_TYPE_SYMLINK:
		mode |= S_IFLNK;
		break;
	case SQSH_INODE_TYPE_BLOCK:
		mode |= S_IFBLK;
		break;
	case SQSH_INODE_TYPE_CHAR:
		mode |= S_IFCHR;
		break;
	case SQSH_INODE_TYPE_FIFO:
		mode |= S_IFIFO;
		break;
	case SQSH_INODE_TYPE_SOCKET:
		mode |= S_IFSOCK;
		break;
	case SQSH_INODE_TYPE_UNKNOWN:
		return 0;
	}
	return mode;
}

static void
sqshfs_inode_to_stat(
		struct SqshInode *inode, const struct SqshSuperblock *superblock,
		struct stat *st) {
	st->st_dev = 0;
	st->st_ino = inode_sqsh_to_fuse(sqsh_inode_number(inode));
	st->st_mode = sqshfs_inode_mode(inode);
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

static struct SqshInode *
sqshfs_inode_open(struct Sqshfs *context, fuse_ino_t ino, int *err) {
	const uint64_t inode_ref = sqshfs_context_inode_ref(context, ino);
	return sqsh_inode_new(context->archive, inode_ref, err);
}

static void
sqshfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	(void)fi;
	int rv = 0;
	struct SqshInode *inode = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	const struct SqshSuperblock *superblock =
			sqsh_archive_superblock(context->archive);

	dbg("sqshfs_getattr\n");

	inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_getattr: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	struct stat stbuf = {0};
	sqshfs_inode_to_stat(inode, superblock, &stbuf);

	dbg("sqshfs_getattr: reply success\n");
	fuse_reply_attr(req, &stbuf, 1.0);
out:
	sqsh_inode_free(inode);
}

static void
sqshfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
	int rv = 0;
	struct SqshDirectoryIterator *iterator = NULL;
	struct SqshInode *parent_inode = NULL;
	struct SqshInode *inode = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	const struct SqshSuperblock *superblock =
			sqsh_archive_superblock(context->archive);

	dbg("sqshfs_lookup: parent_inode: %lu name: %s\n", parent, name);

	parent_inode = sqshfs_inode_open(context, parent, &rv);
	if (rv < 0) {
		dbg("sqshfs_lookup: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	iterator = sqsh_directory_iterator_new(parent_inode, &rv);

	rv = sqsh_directory_iterator_lookup(iterator, name, strlen(name));
	if (rv < 0) {
		dbg("sqshfs_lookup: sqsh_directory_iterator_lookup failed\n");
		fuse_reply_err(req, ENOENT);
		goto out;
	}

	const uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);
	inode = sqsh_inode_new(context->archive, inode_ref, &rv);
	if (rv < 0) {
		dbg("sqshfs_lookup: sqsh_inode_new failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	uint32_t inode_number = sqsh_inode_number(inode);
	if (inode_number > sqsh_superblock_inode_count(superblock)) {
		dbg("sqshfs_lookup: inode %i is too high\n", inode_number);
		fuse_reply_err(req, EIO);
		goto out;
	}

	struct fuse_entry_param entry = {
			.ino = inode_sqsh_to_fuse(inode_number),
			.attr_timeout = 1.0,
			.entry_timeout = 1.0,
			.generation = 1,
	};
	sqshfs_inode_to_stat(inode, NULL, &entry.attr);
	dbg("sqshfs_lookup: reply success. inode: %i\n", inode_number);
	fuse_reply_entry(req, &entry);

	context->inode_map[inode_number - 1] = inode_ref;

out:
	sqsh_inode_free(inode);
	sqsh_directory_iterator_free(iterator);
	sqsh_inode_free(parent_inode);
}

static void
sqshfs_readlink(fuse_req_t req, fuse_ino_t ino) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	dbg("sqshfs_readlink: %i\n", ino);

	inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_readlink: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}
	if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_SYMLINK) {
		dbg("sqshfs_readlink: inode is not a symlink\n");
		fuse_reply_err(req, EINVAL);
		goto out;
	}

	char *symlink = NULL;
	// fuse expects a null terminated string. Duplicate the symlink is the
	// easiest way to do this.
	symlink = sqsh_inode_symlink_dup(inode);
	if (symlink == NULL) {
		dbg("sqshfs_readlink: sqsh_inode_symlink_dup failed\n");
		fuse_reply_err(req, ENOMEM);
		goto out;
	}
	dbg("sqshfs_readlink: reply success\n");
	fuse_reply_readlink(req, symlink);
	free(symlink);

out:
	sqsh_inode_free(inode);
}

static void
sqshfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	int rv = 0;
	struct Sqshfs *context = fuse_req_userdata(req);
	struct SqshfsDirHandle *handle = calloc(1, sizeof(*handle));
	dbg("sqshfs_opendir\n");

	if (handle == NULL) {
		dbg("sqshfs_opendir: calloc failed\n");
		fuse_reply_err(req, ENOMEM);
		goto out;
	}

	handle->inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_opendir: sqsh_inode_new failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}
	handle->iterator = sqsh_directory_iterator_new(handle->inode, &rv);
	if (rv < 0) {
		dbg("sqshfs_opendir: sqsh_directory_iterator_new failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	fi->fh = (uintptr_t)handle;
	dbg("sqshfs_opendir: reply success\n");
	fuse_reply_open(req, fi);
out:
	if (rv < 0) {
		sqsh_inode_free(handle->inode);
		sqsh_directory_iterator_free(handle->iterator);
		free(handle);
	}
}

static void
sqshfs_readdir(
		fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	(void)ino;

	int rv = 0;
	struct SqshInode *inode = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	struct SqshfsDirHandle *handle = (void *)fi->fh;
	dbg("sqshfs_readdir\n");
	char buf[size];

	rv = sqsh_directory_iterator_next(handle->iterator);
	if (rv < 0) {
		dbg("sqshfs_readdir: sqsh_directory_iterator_next failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	} else if (rv == 0) {
		dbg("sqshfs_readdir: end of directory\n");
		fuse_reply_buf(req, NULL, 0);
		goto out;
	}
	const uint64_t inode_ref =
			sqsh_directory_iterator_inode_ref(handle->iterator);
	inode = sqsh_inode_new(context->archive, inode_ref, &rv);
	struct stat stbuf = {0};
	sqshfs_inode_to_stat(inode, NULL, &stbuf);
	char *name = NULL;
	name = sqsh_directory_iterator_name_dup(handle->iterator);
	if (name == NULL) {
		dbg("sqshfs_readdir: sqsh_directory_iterator_name_dup failed\n");
		fuse_reply_err(req, ENOMEM);
		goto out;
	}
	size_t result_size =
			fuse_add_direntry(req, buf, size, name, &stbuf, offset);
	free(name);

	dbg("sqshfs_readdir: reply success\n");
	fuse_reply_buf(req, buf, result_size);

out:
	sqsh_inode_free(inode);
	return;
}

static void
sqshfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	(void)ino;
	struct Sqshfs *context = fuse_req_userdata(req);
	dbg("sqshfs_releasedir\n");
	struct SqshfsDirHandle *handle = (void *)fi->fh;

	sqsh_inode_free(handle->inode);
	sqsh_directory_iterator_free(handle->iterator);
	free(handle);
}

static void
sqshfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	struct Sqshfs *context = fuse_req_userdata(req);
	dbg("sqshfs_open\n");

	int rv = 0;
	struct SqshfsFileHandle *handle = calloc(1, sizeof(*handle));
	if (handle == NULL) {
		dbg("sqshfs_open: calloc failed\n");
		fuse_reply_err(req, ENOMEM);
		goto out;
	}

	handle->inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_open: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}
	fi->fh = (uintptr_t)handle;
	handle = NULL;
	dbg("sqshfs_open: reply success\n");
	fuse_reply_open(req, fi);
out:
	if (handle != NULL) {
		sqsh_inode_free(handle->inode);
		free(handle);
	}
}

static void
sqshfs_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
	(void)ino;
	struct Sqshfs *context = fuse_req_userdata(req);
	struct SqshfsFileHandle *handle = (void *)fi->fh;
	dbg("sqshfs_release\n");

	sqsh_inode_free(handle->inode);
	free(handle);
}

static void
sqshfs_read(
		fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	struct Sqshfs *context = fuse_req_userdata(req);
	struct SqshfsFileHandle *handle = (void *)fi->fh;
	int rv = 0;
	struct SqshFileReader *reader = sqsh_file_reader_new(handle->inode, &rv);

	dbg("sqshfs_read: %i %u %u\n", ino, offset, size);

	uint64_t file_size = sqsh_inode_file_size(handle->inode);
	if (size > file_size - offset) {
		dbg("sqshfs_read: truncating read to file size\n");
		size = file_size - offset;
	}

	rv = sqsh_file_reader_advance(reader, offset, size);
	if (rv < 0) {
		dbg("sqshfs_read: sqsh_file_reader_advance failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}
	const uint8_t *data = sqsh_file_reader_data(reader);
	const size_t data_size = sqsh_file_reader_size(reader);
	fuse_reply_buf(req, (const char *)data, data_size);
out:
	sqsh_file_reader_free(reader);
}

static void
sqshfs_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
	int rv = 0;
	struct SqshInode *inode = NULL;
	struct SqshXattrIterator *iterator = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	const char *value_ptr = NULL;
	size_t value_size;
	dbg("sqshfs_getxattr: %s\n", name);

	inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_getxattr: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	iterator = sqsh_xattr_iterator_new(inode, &rv);
	if (rv < 0) {
		dbg("sqshfs_getxattr: sqsh_xattr_iterator_new failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	while (value_ptr == NULL && (rv = sqsh_xattr_iterator_next(iterator)) > 0) {
		if (sqsh_xattr_iterator_fullname_cmp(iterator, name) == 0) {
			value_ptr = sqsh_xattr_iterator_value(iterator);
			value_size = sqsh_xattr_iterator_value_size(iterator);
		}
	}
	if (rv < 0) {
		fuse_reply_err(req, EIO);
		goto out;
	} else if (value_ptr == NULL) {
		fuse_reply_err(req, ENODATA);
		goto out;
	} else if (size == 0) {
		fuse_reply_xattr(req, value_size);
	} else if (value_size <= size) {
		fuse_reply_buf(req, value_ptr, value_size);
	} else {
		fuse_reply_err(req, ERANGE);
		goto out;
	}

out:
	sqsh_xattr_iterator_free(iterator);
	sqsh_inode_free(inode);
}

static void
sqshfs_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
	(void)req;
	(void)ino;
	(void)size;
	int rv = 0;
	struct SqshInode *inode = NULL;
	struct SqshXattrIterator *iterator = NULL;
	struct Sqshfs *context = fuse_req_userdata(req);
	const char *prefix, *name = NULL;
	char buf[size];
	size_t buf_size = 0;
	char *p;
	dbg("sqshfs_listxattr\n");

	inode = sqshfs_inode_open(context, ino, &rv);
	if (rv < 0) {
		dbg("sqshfs_listxattr: sqshfs_inode_open failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	iterator = sqsh_xattr_iterator_new(inode, &rv);
	if (rv < 0) {
		dbg("sqshfs_listxattr: sqsh_xattr_iterator_new failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}

	p = buf;
	while ((rv = sqsh_xattr_iterator_next(iterator)) > 0) {
		prefix = sqsh_xattr_iterator_prefix(iterator);
		if (prefix == NULL) {
			dbg("sqshfs_listxattr: sqsh_xattr_iterator_prefix failed\n");
			fuse_reply_err(req, EIO);
			goto out;
		}
		name = sqsh_xattr_iterator_name(iterator);
		if (name == NULL) {
			dbg("sqshfs_listxattr: sqsh_xattr_iterator_name failed\n");
			fuse_reply_err(req, EIO);
			goto out;
		}

		size_t prefix_size = sqsh_xattr_iterator_prefix_size(iterator);
		buf_size += prefix_size;
		if (buf_size < size) {
			memcpy(p, prefix, prefix_size);
			p = &buf[buf_size];
		}

		size_t name_size = sqsh_xattr_iterator_name_size(iterator);
		buf_size += name_size + 1;
		if (buf_size < size) {
			memcpy(p, name, name_size);
			p[name_size] = '\0';
			p = &buf[buf_size];
		}
	}
	if (rv < 0) {
		dbg("sqshfs_listxattr: sqsh_xattr_iterator_next failed\n");
		fuse_reply_err(req, EIO);
		goto out;
	}
	if (size == 0) {
		dbg("sqshfs_listxattr: replying with size %zu", buf_size);
		fuse_reply_xattr(req, buf_size);
	} else {
		dbg("sqshfs_listxattr: replying with buffer size %zu", buf_size);
		fuse_reply_buf(req, buf, buf_size);
	}
out:
	sqsh_xattr_iterator_free(iterator);
	sqsh_inode_free(inode);
}

#if 0
static void
sqshfs_lseek(
		fuse_req_t req, fuse_ino_t ino, off_t off, int whence,
		struct fuse_file_info *fi) {
	dbg("sqshfs_lseek\n");
	struct SqshfsFileHandle *handle = (void *)fi->fh;
	int rv = 0;
	sqsh_file_reader_free(handle->reader);
	handle->reader = sqsh_file_reader_new(handle->inode, &rv);
	handle->last_size = off;
}
#endif

static const struct fuse_lowlevel_ops sqshfs_oper = {
		.init = sqshfs_init,
		.destroy = sqshfs_destroy,
		.lookup = sqshfs_lookup,
		.getattr = sqshfs_getattr,
		.readlink = sqshfs_readlink,
		.opendir = sqshfs_opendir,
		.readdir = sqshfs_readdir,
		.releasedir = sqshfs_releasedir,
		.open = sqshfs_open,
		.release = sqshfs_release,
		.read = sqshfs_read,
		.getxattr = sqshfs_getxattr,
		.listxattr = sqshfs_listxattr,
		//.lseek = sqshfs_lseek,
};

static int
sqshfs_process_options(
		void *data, const char *arg, int key, struct fuse_args *outargs) {
	(void)outargs;
	struct SqshfsOptions *sqshfs_options = data;
	if (key == FUSE_OPT_KEY_NONOPT && sqshfs_options->archive == NULL) {
		sqshfs_options->archive = arg;
		return 0;
	}
	return 1;
}

int
main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_session *fuse_session = NULL;
	struct fuse_cmdline_opts fuse_options = {0};
	struct SqshfsOptions sqshfs_options = {0};
	struct fuse_loop_config config = {0};
	struct Sqshfs sqshfs_context = {0};
	int rv = EXIT_SUCCESS;

	if (fuse_opt_parse(
				&args, &sqshfs_options, sqshfs_opts, sqshfs_process_options) ==
		-1) {
		usage(argv[0]);
	}

	if (fuse_parse_cmdline(&args, &fuse_options) != 0) {
		return 1;
	}
	if (fuse_options.show_help) {
		usage(argv[0]);
		help();
		fuse_lowlevel_help();
		fuse_cmdline_help();
		rv = EXIT_FAILURE;
		goto out;
	} else if (fuse_options.show_version) {
		printf("%s-" VERSION "\n", argv[0]);
		fuse_lowlevel_version();
		rv = EXIT_FAILURE;
		goto out;
	}

	if (sqshfs_options.archive == NULL || fuse_options.mountpoint == NULL) {
		usage(argv[0]);
		goto out;
	}

	pthread_mutex_init(&sqshfs_context.lock, NULL);
	sqshfs_context.archive = open_archive(sqshfs_options.archive, &rv);
	if (rv < 0) {
		sqsh_perror(rv, sqshfs_options.archive);
		rv = EXIT_FAILURE;
		goto out;
	}

	sqshfs_context.debug = fuse_options.debug;
	fuse_session = fuse_session_new(
			&args, &sqshfs_oper, sizeof(sqshfs_oper), &sqshfs_context);
	if (fuse_session == NULL) {
		goto out;
	}

	const struct SqshSuperblock *superblock =
			sqsh_archive_superblock(sqshfs_context.archive);
	const uint64_t inode_count = sqsh_superblock_inode_count(superblock);

	sqshfs_context.inode_map = calloc(inode_count, sizeof(uint64_t));
	if (sqshfs_context.inode_map == NULL) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (fuse_set_signal_handlers(fuse_session) != 0) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (fuse_session_mount(fuse_session, fuse_options.mountpoint) != 0) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (fuse_daemonize(fuse_options.foreground) != 0) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (fuse_options.singlethread)
		rv = fuse_session_loop(fuse_session);
	else {
		config.clone_fd = fuse_options.clone_fd;
		config.max_idle_threads = fuse_options.max_idle_threads;
		rv = fuse_session_loop_mt(fuse_session, &config);
	}

	fuse_session_unmount(fuse_session);

out:
	if (fuse_session != NULL) {
		fuse_remove_signal_handlers(fuse_session);
		fuse_session_destroy(fuse_session);
	}
	free(fuse_options.mountpoint);
	fuse_opt_free_args(&args);
	free(sqshfs_context.inode_map);
	sqsh_archive_free(sqshfs_context.archive);

	return rv;
}
