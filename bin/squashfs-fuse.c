/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : squashfs-fuse
 * @created     : Wednesday Sep 08, 2021 09:06:25 CEST
 */

#define FUSE_USE_VERSION 31

#include <errno.h>
#include <fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/resolve_path.h"
#include "../src/squash.h"

static struct { struct Squash squash; } data = {0};

static struct SquashfuseOptions {
	int show_help;
	const char *image_path;
} options = {0};

#define SQUASH_OPT_KEY(t, p) \
	{ t, offsetof(struct SquashfuseOptions, p), 1 }
// clang-format off
static const struct fuse_opt option_spec[] = {
	SQUASH_OPT_KEY("-h", show_help),
	SQUASH_OPT_KEY("--help", show_help),
	FUSE_OPT_END
};
// clang-format on

static void
help(const char *arg0) {
	// TODO
}

static void *
squashfuse_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
	int rv = 0;

	rv = squash_open(&data.squash, options.image_path);
	if (rv < 0) {
		perror(options.image_path);
	}

	cfg->kernel_cache = 1;
	return NULL;
}

static int
squashfuse_getattr(
		const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	int rv = 0;
	memset(stbuf, 0, sizeof(struct stat));

	struct SquashInodeContext inode = {0};

	rv = squash_resolve_path(&inode, data.squash.superblock, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	stbuf->st_mode = squash_inode_permission(&inode);
	stbuf->st_nlink = squash_inode_hard_link_count(&inode);
	stbuf->st_size = squash_inode_file_size(&inode);
	stbuf->st_mtime = stbuf->st_ctime = stbuf->st_atime =
			squash_inode_modified_time(&inode);
	switch (squash_inode_type(&inode)) {
	case SQUASH_INODE_TYPE_DIRECTORY:
		stbuf->st_mode |= S_IFDIR;
		break;
	case SQUASH_INODE_TYPE_FILE:
		stbuf->st_mode |= S_IFREG;
		break;
	case SQUASH_INODE_TYPE_SYMLINK:
		stbuf->st_mode |= S_IFLNK;
		break;
	case SQUASH_INODE_TYPE_BLOCK:
		stbuf->st_mode |= S_IFBLK;
		break;
	case SQUASH_INODE_TYPE_CHAR:
		stbuf->st_mode |= S_IFCHR;
		break;
	case SQUASH_INODE_TYPE_FIFO:
		stbuf->st_mode |= S_IFIFO;
		break;
	case SQUASH_INODE_TYPE_SOCKET:
		stbuf->st_mode |= S_IFSOCK;
		break;
	case SQUASH_INODE_TYPE_UNKNOWN:
		rv = -EIO;
		goto out;
	}
out:
	squash_inode_cleanup(&inode);
	return rv;
}

static int
squashfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi,
		enum fuse_readdir_flags flags) {
	int rv = 0;
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	const struct SquashDirectoryEntry *entry;
	rv = squash_resolve_path(&inode, data.squash.superblock, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	rv = squash_directory_init(&dir, data.squash.superblock, &inode);
	if (rv < 0) {
		rv = -ENOMEM;
		goto out;
	}
	rv = squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		rv = -ENOMEM;
		goto out;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);

	while ((entry = squash_directory_iterator_next(&iter))) {
		char *name;
		rv = squash_directory_entry_name_dup(entry, &name);
		if (rv < 0) {
			rv = -ENOMEM;
			goto out;
		}
		rv = filler(buf, name, NULL, 0, 0);
		if (rv < 0) {
			rv = -ENOMEM;
			goto out;
		}
	}

out:
	squash_directory_iterator_cleanup(&iter);
	squash_directory_cleanup(&dir);
	squash_inode_cleanup(&inode);
	return rv;
}

static int
squashfuse_open(const char *path, struct fuse_file_info *fi) {
	int rv = 0;
	struct SquashInodeContext inode = {0};

	rv = squash_resolve_path(&inode, data.squash.superblock, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		rv = -EACCES;
		goto out;
	}

out:
	squash_inode_cleanup(&inode);
	return rv;
}

static int
squashfuse_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	int rv = 0;
	struct SquashInodeContext inode = {0};

	rv = squash_resolve_path(&inode, data.squash.superblock, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}
	memset(buf, 'a', size);
	rv = size;
out:
	squash_inode_cleanup(&inode);
	return rv;
}

static int
squashfuse_readlink(const char *path, char *buf, size_t size) {
	int rv = 0;
	struct SquashInodeContext inode = {0};

	rv = squash_resolve_path(&inode, data.squash.superblock, path);
	if (rv < 0) {
		rv = -ENOENT;
		goto out;
	}

	const char *symlink = squash_inode_symlink(&inode);
	size_t symlink_size = squash_inode_symlink_size(&inode);
	size_t cpy_size = MIN(symlink_size, size - 1);

	memcpy(buf, symlink, cpy_size);
	buf[cpy_size] = 0;

out:
	squash_inode_cleanup(&inode);
	return rv;
}

static const struct fuse_operations squashfuse_operations = {
		.init = squashfuse_init,
		.getattr = squashfuse_getattr,
		.readdir = squashfuse_readdir,
		.open = squashfuse_open,
		.read = squashfuse_read,
		.readlink = squashfuse_readlink,
};

static int
squashfuse_process_options(
		void *data, const char *arg, int key, struct fuse_args *outargs) {
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
	if (fuse_opt_parse(&args, &options, option_spec,
				squashfuse_process_options) == -1) {
		rv = EXIT_FAILURE;
		goto out;
	}

	if (options.show_help) {
		help(argv[0]);
		fuse_opt_add_arg(&args, "--help");
		args.argv[0][0] = '\0';
	}

	rv = fuse_main(args.argc, args.argv, &squashfuse_operations, &data);
out:
	return rv;
}
