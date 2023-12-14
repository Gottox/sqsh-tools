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
 * @file         unpack.c
 */

#include <sqshtools_common.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

bool do_chown = false;
bool verbose = false;

struct PathStack {
	const char *segment;
	const struct PathStack *prev;
};

static int
extract(const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack);

static int
usage(char *arg0) {
	printf("usage: %s [-o OFFSET] [-cV] FILESYSTEM [PATH] [TARGET DIR]\n",
		   arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static void
print_path(const struct PathStack *path_stack, char *suffix, FILE *stream) {
	if (path_stack == NULL) {
		return;
	}
	if (strcmp(path_stack->segment, ".") == 0) {
		print_path(path_stack->prev, suffix, stream);
		return;
	}
	print_path(path_stack->prev, "/", stream);
	fputs(path_stack->segment, stream);
	fputs(suffix, stream);
}

static void
print_err(int rv, const char *module, const struct PathStack *path_stack) {
	fputs(module, stderr);
	fputs(": ", stderr);
	print_path(path_stack, "", stderr);
	sqsh_perror(rv, "");
}

static int
extract_directory_entry(
		struct SqshDirectoryIterator *iter,
		const struct PathStack *path_stack) {
	int rv;
	struct SqshFile *file = NULL;
	size_t size;
	const char *filename_ptr = sqsh_directory_iterator_name2(iter, &size);
	char filename[size + 1];
	memcpy(filename, filename_ptr, size);
	filename[size] = '\0';

	file = sqsh_directory_iterator_open_file(iter, &rv);
	if (rv < 0) {
		print_err(rv, "sqsh_directory_iterator_open_file", path_stack);
		goto out;
	}
	rv = extract(filename, file, path_stack);
out:
	sqsh_close(file);
	return rv;
}

static int
extract_directory(
		const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack) {
	int rv;
	char cwd[PATH_MAX] = {0};
	struct SqshDirectoryIterator *iter = NULL;

	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		print_err(rv = -errno, "getcwd", path_stack);
		goto out;
	}
	rv = mkdir(filename, 0700);
	if (rv < 0 && errno != EEXIST) {
		print_err(rv = -errno, "mkdir", path_stack);
		goto out;
	}
	rv = chdir(filename);
	if (rv < 0) {
		print_err(rv = -errno, "chdir", path_stack);
		goto out;
	}

	iter = sqsh_directory_iterator_new(file, &rv);
	if (rv < 0) {
		print_err(rv, "sqsh_directory_iterator_new", path_stack);
		goto out;
	}

	while (sqsh_directory_iterator_next(iter, &rv)) {
		rv = extract_directory_entry(iter, path_stack);
		// Don't stop on error, but continue with next entry.
		/*if (rv < 0) {
			goto out;
		}*/
	}
	if (rv < 0) {
		print_err(rv, "sqsh_directory_iterator_next", path_stack);
		goto out;
	}

	rv = chdir(cwd);
	if (rv < 0) {
		print_err(rv = -errno, "chdir", path_stack->prev);
		goto out;
	}
out:
	sqsh_directory_iterator_free(iter);
	return rv;
}

static int
extract_file(
		const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack) {
	int rv = 0;
	FILE *stream = NULL;
	char tmp_filename[] = ".sqsh-unpack-XXXXXX";

	int fd = mkstemp(tmp_filename);
	if (fd < 0) {
		print_err(rv = -errno, "mkstemp", path_stack);
		goto out;
	}
	stream = fdopen(fd, "w");
	if (stream == NULL) {
		print_err(rv = -errno, "fdopen", path_stack);
		goto out;
	}

	rv = sqsh_file_to_stream(file, stream);
	if (rv < 0) {
		print_err(rv, "sqsh_file_to_stream", path_stack);
		rv = EXIT_FAILURE;
		goto out;
	}
	fclose(stream);

	rv = rename(tmp_filename, filename);
	if (rv < 0 && errno != ENOENT) {
		print_err(rv = -errno, "rename", path_stack);
		goto out;
	}
out:
	return rv;
}

static int
extract_symlink(
		const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack) {
	int rv;
	const char *target_ptr = sqsh_file_symlink(file);
	size_t size = sqsh_file_symlink_size(file);
	char target[size + 1];
	memcpy(target, target_ptr, size);
	target[size] = '\0';

	rv = symlink(target, filename);
	if (rv < 0) {
		print_err(rv = -errno, "symlink", path_stack);
		goto out;
	}

out:
	return rv;
}

static int
extract_device(
		const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack) {
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
		print_err(-EINVAL, "extract_device", path_stack);
		rv = -EINVAL;
		goto out;
	}

	rv = mknod(filename, mode, sqsh_file_device_id(file));
	if (rv < 0) {
		print_err(rv = -errno, "mknod", path_stack);
		goto out;
	}
out:
	return rv;
}

static int
extract(const char *filename, struct SqshFile *file,
		const struct PathStack *path_stack) {
	const struct PathStack new_path_stack = {
			.segment = filename, .prev = path_stack};
	int rv = 0;
	uint32_t fuid, fgid;
	const enum SqshFileType type = sqsh_file_type(file);
	const uint16_t mode = sqsh_file_permission(file);
	struct utimbuf times;

	if (verbose) {
		print_path(&new_path_stack, "\n", stderr);
	}

	if (type != SQSH_FILE_TYPE_DIRECTORY) {
		rv = unlink(filename);
		if (rv < 0 && errno != ENOENT) {
			print_err(rv = -errno, "unlink", path_stack);
			goto out;
		}
	}

	switch (type) {
	case SQSH_FILE_TYPE_DIRECTORY:
		rv = extract_directory(filename, file, &new_path_stack);
		break;
	case SQSH_FILE_TYPE_FILE:
		rv = extract_file(filename, file, &new_path_stack);
		break;
	case SQSH_FILE_TYPE_SYMLINK:
		rv = extract_symlink(filename, file, &new_path_stack);
		break;
	case SQSH_FILE_TYPE_BLOCK:
	case SQSH_FILE_TYPE_CHAR:
	case SQSH_FILE_TYPE_FIFO:
	case SQSH_FILE_TYPE_SOCKET:
		rv = extract_device(filename, file, &new_path_stack);
		break;
	default:
		print_err(-EINVAL, "extract", &new_path_stack);
		rv = -EINVAL;
	}
	if (rv < 0) {
		goto out;
	}

	if (type != SQSH_FILE_TYPE_SYMLINK) {
		times.actime = times.modtime = sqsh_file_modified_time(file);
		rv = utime(filename, &times);
		if (rv < 0) {
			print_err(rv = -errno, "utime", &new_path_stack);
			goto out;
		}

		rv = fchmodat(AT_FDCWD, filename, mode, AT_SYMLINK_NOFOLLOW);
		if (rv < 0) {
			print_err(rv = -errno, "chmod", path_stack);
			goto out;
		}
	}

	if (do_chown) {
		fuid = sqsh_file_uid(file);
		fgid = sqsh_file_gid(file);
		rv = fchownat(AT_FDCWD, filename, fuid, fgid, AT_SYMLINK_NOFOLLOW);
		if (rv < 0) {
			print_err(rv = -errno, "chown", &new_path_stack);
			goto out;
		}
	}

out:
	return rv;
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

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *image_path;
	char *src_path = "/";
	char *target_path = NULL;
	struct SqshArchive *sqsh;
	struct SqshPathResolver *resolver = NULL;
	struct SqshFile *file = NULL;
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
	resolver = sqsh_path_resolver_new(sqsh, &rv);
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = sqsh_path_resolver_resolve(resolver, src_path, false);
	if (rv < 0) {
		sqsh_perror(rv, src_path);
		rv = EXIT_FAILURE;
		goto out;
	}
	file = sqsh_path_resolver_open_file(resolver, &rv);
	if (rv < 0) {
		sqsh_perror(rv, src_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	if (target_path == NULL) {
		target_path = basename(src_path);
		if (strcmp(target_path, "/") == 0) {
			target_path = ".";
		}
	} else if (sqsh_file_type(file) != SQSH_FILE_TYPE_DIRECTORY) {
		struct stat st = {0};
		rv = stat(target_path, &st);
		if (rv < 0) {
			perror(target_path);
			rv = EXIT_FAILURE;
			goto out;
		}
		if (S_ISDIR(st.st_mode)) {
			rv = chdir(target_path);
			if (rv < 0) {
				perror(target_path);
				rv = EXIT_FAILURE;
				goto out;
			}
			target_path = basename(src_path);
			if (strcmp(target_path, "/") == 0) {
				target_path = ".";
			}
		}
	}

	rv = extract(target_path, file, NULL);
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}
out:
	sqsh_close(file);
	sqsh_path_resolver_free(resolver);
	sqsh_archive_close(sqsh);
	return rv;
}
