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
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

typedef int (*extract_fn)(
		const char *, enum SqshFileType, const struct SqshFile *);

size_t max_open_files = 0;
size_t extracted_files = 0;
bool do_chown = false;
bool verbose = false;
const char *image_path;
struct SqshThreadpool *threadpool;

static void (*print_segment)(const char *segment, size_t segment_size) =
		print_raw;

static int
usage(char *arg0) {
	printf("usage: %s [-o OFFSET] [-cVRe] FILESYSTEM [PATH] [TARGET DIR]\n",
		   arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static const char opts[] = "co:vVhRe";
static const struct option long_opts[] = {
		{"chown", no_argument, NULL, 'c'},
		{"offset", required_argument, NULL, 'o'},
		{"version", no_argument, NULL, 'v'},
		{"verbose", no_argument, NULL, 'V'},
		{"help", no_argument, NULL, 'h'},
		{"raw", no_argument, NULL, 'R'},
		{"escape", no_argument, NULL, 'e'},
		{0},
};

static int
update_metadata(const char *path, const struct SqshFile *file) {
	int rv = 0;
	struct timespec times[2] = {0};

	times[0].tv_sec = times[1].tv_sec = sqsh_file_modified_time(file);
	rv = utimensat(AT_FDCWD, path, times, AT_SYMLINK_NOFOLLOW);
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
extract_dir(const char *path) {
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
update_metadata_dir(const char *path, const struct SqshFile *file) {
	// The directory should already be created by prepare_dir(), so we just
	// apply the permissions.
	int rv = 0;
	rv = chmod(path, sqsh_file_permission(file));
	if (rv < 0) {
		perror(path);
		goto out;
	}

	rv = update_metadata(path, file);
out:
	return rv;
}

struct ExtractFileData {
	char tmp_filename[32];
	char *path;
};

static void
extract_file_cleanup(struct ExtractFileData *data, FILE *stream) {
	if (data == NULL) {
		return;
	}
	if (stream != NULL) {
		fclose(stream);
	}
	free(data->path);
	free(data);
}
static void
extract_file_after(
		const struct SqshFile *file, FILE *stream, void *d, int err) {
	(void)stream;
	int rv = 0;
	struct ExtractFileData *data = d;
	if (err < 0) {
		sqsh_perror(err, data->path);
	}

	rv = rename(data->tmp_filename, data->path);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	fclose(stream);
	rv = update_metadata(data->path, file);
out:
	if (rv < 0) {
		sqsh_perror(rv, data->path);
	}
	extract_file_cleanup(data, NULL);
}

static int
extract_file(const char *path, const struct SqshFile *file) {
	int rv = 0;
	FILE *stream = NULL;
	struct ExtractFileData *data = calloc(1, sizeof(struct ExtractFileData));
	if (data == NULL) {
		rv = -errno;
		goto out;
	}
	data->path = strdup(path);
	if (data->path == NULL) {
		rv = -errno;
		perror(path);
		goto out;
	}
	strcpy(data->tmp_filename, ".sqsh-unpack-XXXXXX");

	int fd = mkstemp(data->tmp_filename);
	if (fd < 0) {
		rv = -errno;
		perror(path);
		goto out;
	}

	rv = ftruncate(fd, sqsh_file_size(file));
	if (rv < 0) {
		rv = -errno;
		perror(path);
		goto out;
	}

	stream = fdopen(fd, "w");
	if (stream == NULL) {
		rv = -errno;
		perror(path);
		goto out;
	}
	fd = -1;

	extracted_files++;
	if (extracted_files % max_open_files == 0) {
		rv = sqsh_threadpool_wait(threadpool);
		if (rv < 0) {
			goto out;
		}
	}

	sqsh_file_to_stream_mt(file, threadpool, stream, extract_file_after, data);
	// rv = sqsh_file_to_stream(file, stream);
	// if (rv < 0) {
	//	goto out;
	// }
	// extract_file_after(file, stream, data, rv);
out:
	if (rv < 0) {
		sqsh_perror(rv, path);
		extract_file_cleanup(data, stream);
		if (fd > 0) {
			close(fd);
		}
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

	rv = update_metadata(path, file);
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

	rv = update_metadata(path, file);
out:
	return rv;
}

static int
extract_first_pass(
		const char *path, enum SqshFileType type, const struct SqshFile *file) {
	switch (type) {
	case SQSH_FILE_TYPE_DIRECTORY:
		return 0;
	case SQSH_FILE_TYPE_FILE:
		return extract_file(path, file);
	case SQSH_FILE_TYPE_SYMLINK:
		return extract_symlink(path, file);
	case SQSH_FILE_TYPE_BLOCK:
	case SQSH_FILE_TYPE_CHAR:
	case SQSH_FILE_TYPE_FIFO:
	case SQSH_FILE_TYPE_SOCKET:
		return extract_device(path, file);
	default:
		__builtin_unreachable();
	}
}

static int
extract_second_pass(
		const char *path, enum SqshFileType type, const struct SqshFile *file) {
	if (type == SQSH_FILE_TYPE_DIRECTORY) {
		return update_metadata_dir(path, file);
	} else {
		return 0;
	}
}

static int
extract(const char *path, const struct SqshFile *file, extract_fn func) {
	if (path[0] == 0) {
		path = ".";
	}

	enum SqshFileType type = sqsh_file_type(file);
	return func(path, type, file);
}

static int
extract_from_traversal(
		const char *target_path, const struct SqshTreeTraversal *iter,
		extract_fn func) {
	int rv;
	char *path = sqsh_tree_traversal_path_dup(iter);
	enum SqshTreeTraversalState state = sqsh_tree_traversal_state(iter);
	struct SqshFile *file = NULL;

	if (verbose && state != SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END) {
		print_segment(target_path, strlen(target_path));
		if (path[0] != 0) {
			print_segment("/", 1);
			print_segment(path, strlen(path));
		}
		puts("");
	}

	if (state == SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN) {
		// In case we hit a directory, we create it first. The directory meta
		// data will be set later
		rv = extract_dir(path);
		if (rv < 0) {
			goto out;
		}
	} else {
		file = sqsh_tree_traversal_open_file(iter, &rv);
		if (rv < 0) {
			sqsh_perror(rv, path);
			goto out;
		}

		rv = extract(path, file, func);
		// Ignore errors, we want to extract as much as possible.
		rv = 0;
	}
out:
	sqsh_close(file);
	free(path);
	return rv;
}

static int
extract_all(
		const char *target_path, const struct SqshFile *base, extract_fn func) {
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
		rv = extract_from_traversal(target_path, iter, func);
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
	struct rlimit limits = {0};
	if (isatty(STDOUT_FILENO)) {
		print_segment = print_escaped;
	}

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
		case 'R':
			print_segment = print_raw;
			break;
		case 'e':
			print_segment = print_escaped;
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

	threadpool = sqsh_threadpool_new(0, &rv);

	rv = getrlimit(RLIMIT_NOFILE, &limits);
	if (rv < 0) {
		perror("getrlimit");
		rv = EXIT_FAILURE;
		goto out;
	}
	max_open_files = limits.rlim_cur / 2;

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
		rv = extract(target_path, src_root, extract_first_pass);
	} else {
		rv = extract_all(target_path, src_root, extract_first_pass);
	}
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}
	rv = sqsh_threadpool_wait(threadpool);
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}
	sqsh_threadpool_free(threadpool);
	threadpool = NULL;

	if (sqsh_file_type(src_root) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = extract(target_path, src_root, extract_second_pass);
	} else {
		rv = extract_all(target_path, src_root, extract_second_pass);
	}
out:
	sqsh_threadpool_free(threadpool);
	sqsh_close(src_root);
	sqsh_archive_close(sqsh);
	return rv;
}
