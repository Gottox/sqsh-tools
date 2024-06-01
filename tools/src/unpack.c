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
 * @author       Enno Boland (mail@eboland.de)
 * @file         unpack.c
 */

#include <sqshtools_common.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

bool do_chown = false;
bool verbose = false;
const char *image_path;

static int
usage(char *arg0) {
	printf("usage: %s [-o OFFSET] [-cV] FILESYSTEM [PATH] [TARGET DIR]\n",
		   arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static const char opts[] = "co:vVh";
static const struct option long_opts[] = {
		{"chown", no_argument, NULL, 'c'},
		{"offset", required_argument, NULL, 'o'},
		{"version", no_argument, NULL, 'v'},
		{"verbose", no_argument, NULL, 'V'},
		{"help", no_argument, NULL, 'h'},
		{0},
};

static int
prepare_dir(const char *path) {
	int rv = 0;
	rv = mkdir(path, 0700);
	if (rv < 0 && errno == EEXIST) {
		struct stat st;
		rv = stat(path, &st);
		if (rv < 0) {
			perror(path);
			goto out;
		}
		if (!S_ISDIR(st.st_mode)) {
			errno = EEXIST;
			perror(path);
			goto out;
		}

	} else if (rv < 0) {
		perror(path);
		goto out;
	}

out:
	return rv;
}

static int
extract_dir(const char *path, const struct SqshFile *file) {
	// The directory should already be created by prepare_dir(), so we just
	// apply the permissions.
	int rv = 0;
	rv = chmod(path, sqsh_file_permission(file));
	if (rv < 0) {
		perror(path);
		goto out;
	}

out:
	return rv;
}

static int
extract_file(const char *path, const struct SqshFile *file) {
	int rv = 0;
	FILE *stream = NULL;
	char tmp_filename[] = ".sqsh-unpack-XXXXXX";

	int fd = mkstemp(tmp_filename);
	if (fd < 0) {
		rv = -errno;
		perror(path);
		goto out;
	}
	stream = fdopen(fd, "w");
	if (stream == NULL) {
		perror(path);
		goto out;
	}
	fd = -1;

	rv = sqsh_file_to_stream(file, stream);
	if (rv < 0) {
		sqsh_perror(rv, path);
		goto out;
	}

	rv = rename(tmp_filename, path);
	if (rv < 0) {
		perror(path);
		goto out;
	}
out:
	if (stream != NULL) {
		fclose(stream);
	}
	if (fd > 0) {
		close(fd);
	}
	return rv;
}

static int
extract_symlink(const char *path, const struct SqshFile *file) {
	int rv;
	char *target = sqsh_file_symlink_dup(file);

	rv = symlink(target, path);
	if (rv < 0) {
		perror(path);
		goto out;
	}

out:
	free(target);
	return rv;
}

static int
extract_device(const char *path, const struct SqshFile *file) {
	int rv = 0;
	uint16_t mode = sqsh_file_permission(file);
	switch (sqsh_file_type(file)) {
	case SQSH_FILE_TYPE_BLOCK:
		mode |= S_IFCHR;
		break;
	case SQSH_FILE_TYPE_CHAR:
		mode |= S_IFBLK;
		break;
	case SQSH_FILE_TYPE_FIFO:
		mode |= S_IFIFO;
		break;
	case SQSH_FILE_TYPE_SOCKET:
		mode |= S_IFSOCK;
		break;
	default:
		rv = errno = -EINVAL;
		perror(path);
		goto out;
	}

	rv = mknod(path, mode, sqsh_file_device_id(file));
	if (rv < 0) {
		perror(path);
		goto out;
	}
out:
	return rv;
}

static int
extract(const char *path, const struct SqshFile *file) {
	int rv = 0;
	struct utimbuf times;

	if (path[0] == 0) {
		path = ".";
	}

	enum SqshFileType type = sqsh_file_type(file);
	switch (type) {
	case SQSH_FILE_TYPE_DIRECTORY:
		rv = extract_dir(path, file);
		break;
	case SQSH_FILE_TYPE_FILE:
		rv = extract_file(path, file);
		break;
	case SQSH_FILE_TYPE_SYMLINK:
		rv = extract_symlink(path, file);
		break;
	case SQSH_FILE_TYPE_BLOCK:
	case SQSH_FILE_TYPE_CHAR:
	case SQSH_FILE_TYPE_FIFO:
	case SQSH_FILE_TYPE_SOCKET:
		rv = extract_device(path, file);
		break;
	default:
		__builtin_unreachable();
	}
	if (rv < 0) {
		sqsh_perror(rv, path);
		goto out;
	}

	if (type != SQSH_FILE_TYPE_SYMLINK) {
		times.actime = times.modtime = sqsh_file_modified_time(file);
		rv = utime(path, &times);
		if (rv < 0) {
			perror(path);
			goto out;
		}

		uint16_t mode = sqsh_file_permission(file);
		rv = fchmodat(AT_FDCWD, path, mode, AT_SYMLINK_NOFOLLOW);
		if (rv < 0) {
			perror(path);
			goto out;
		}
	}

	if (do_chown) {
		const uint32_t uid = sqsh_file_uid(file);
		const uint32_t gid = sqsh_file_gid(file);

		rv = chown(path, uid, gid);
		if (rv < 0) {
			perror(path);
			goto out;
		}
	}

out:
	return rv;
}

static int
extract_from_traversal(
		const char *target_path, const struct SqshTreeTraversal *iter) {
	int rv;
	char *path = sqsh_tree_traversal_path_dup(iter);
	enum SqshTreeTraversalState state = sqsh_tree_traversal_state(iter);
	struct SqshFile *file = NULL;

	if (verbose && state != SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END) {
		fputs(target_path, stdout);
		if (path[0] != 0) {
			fputc('/', stdout);
			fputs(path, stdout);
		}
		fputc('\n', stdout);
	}

	if (state == SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN) {
		// In case we hit a directory, we create it first. The directory meta
		// data will be set later
		rv = prepare_dir(path);
		if (rv < 0) {
			goto out;
		}
	} else {
		file = sqsh_tree_traversal_open_file(iter, &rv);
		if (rv < 0) {
			sqsh_perror(rv, path);
			goto out;
		}

		rv = extract(path, file);
		// Ignore errors, we want to extract as much as possible.
		rv = 0;
	}
out:
	sqsh_close(file);
	free(path);
	return rv;
}

static int
extract_all(const char *target_path, const struct SqshFile *base) {
	char *path = NULL;
	int rv = 0;
	struct SqshTreeTraversal *iter = NULL;
	rv = mkdir(target_path, 0700);
	if (rv < 0 && errno != EEXIST) {
		perror(target_path);
		goto out;
	}

	rv = chdir(target_path);
	if (rv < 0) {
		perror(target_path);
		goto out;
	}

	iter = sqsh_tree_traversal_new(base, &rv);
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		goto out;
	}

	bool has_next = sqsh_tree_traversal_next(iter, &rv);
	if (!has_next) {
		rv = -SQSH_ERROR_INTERNAL;
		sqsh_perror(rv, image_path);
		goto out;
	}

	while (sqsh_tree_traversal_next(iter, &rv)) {
		rv = extract_from_traversal(target_path, iter);
		if (rv < 0) {
			goto out;
		}
	}
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		goto out;
	}

out:
	free(path);
	sqsh_tree_traversal_free(iter);
	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	char *src_path = "/";
	char *target_path = NULL;
	struct SqshArchive *sqsh;
	struct SqshFile *src_root = NULL;
	uint64_t offset = 0;

	while ((opt = getopt_long(argc, argv, opts, long_opts, NULL)) != -1) {
		switch (opt) {
		case 'c':
			do_chown = true;
			break;
		case 'o':
			offset = strtoull(optarg, NULL, 0);
			break;
		case 'v':
			puts("sqsh-unpack-" VERSION);
			return 0;
		case 'V':
			verbose = true;
			break;
		default:
			return usage(argv[0]);
		}
	}

	if (optind >= argc) {
		return usage(argv[0]);
	}

	image_path = argv[optind];

	if (optind + 1 < argc) {
		src_path = argv[optind + 1];
	}
	if (optind + 2 < argc) {
		target_path = argv[optind + 2];
	}

	sqsh = open_archive(image_path, offset, &rv);
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	src_root = sqsh_lopen(sqsh, src_path, &rv);
	if (rv < 0) {
		sqsh_perror(rv, src_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	if (target_path == NULL) {
		target_path = basename(src_path);
		if (target_path[0] == '/' && target_path[1] == 0) {
			target_path = ".";
		}
	}

	if (sqsh_file_type(src_root) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = extract(target_path, src_root);
	} else {
		rv = extract_all(target_path, src_root);
	}
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}
out:
	sqsh_close(src_root);
	sqsh_archive_close(sqsh);
	return rv;
}
