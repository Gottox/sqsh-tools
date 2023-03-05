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

#include "common.h"

#include <sqsh_context.h>
#include <sqsh_directory.h>
#include <sqsh_file.h>

#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool do_chown = false;
bool verbose = false;

struct PathStack {
	const char *segment;
	const struct PathStack *prev;
};

static int
extract(const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack);

static int
usage(char *arg0) {
	printf("usage: %s [-cV] FILESYSTEM [PATH] [TARGET DIR]\n", arg0);
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
	struct SqshInodeContext *inode = NULL;
	const char *filename_ptr = sqsh_directory_iterator_name(iter);
	size_t size = sqsh_directory_iterator_name_size(iter);
	char filename[size + 1];
	memcpy(filename, filename_ptr, size);
	filename[size] = '\0';

	inode = sqsh_directory_iterator_inode_load(iter, &rv);
	if (rv < 0) {
		print_err(rv, "sqsh_directory_iterator_inode_load", path_stack);
		goto out;
	}
	rv = extract(filename, inode, path_stack);
out:
	sqsh_inode_free(inode);
	return rv;
}

static int
extract_directory(
		const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack) {
	int rv;
	char cwd[PATH_MAX] = {0};
	struct SqshDirectoryIterator *iter = NULL;
	uint16_t mode = sqsh_inode_permission(inode);

	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		print_err(rv = -errno, "getcwd", path_stack);
		goto out;
	}
	rv = mkdir(filename, mode);
	if (rv < 0 && errno != EEXIST) {
		print_err(rv = -errno, "mkdir", path_stack);
		goto out;
	}
	rv = chdir(filename);
	if (rv < 0) {
		print_err(rv = -errno, "chdir", path_stack);
		goto out;
	}

	iter = sqsh_directory_iterator_new(inode, &rv);
	if (rv < 0) {
		print_err(rv, "sqsh_directory_iterator_new", path_stack);
		goto out;
	}

	while (sqsh_directory_iterator_next(iter) > 0) {
		rv = extract_directory_entry(iter, path_stack);
		// Don't stop on error, but continue with next entry.
		/*if (rv < 0) {
			goto out;
		}*/
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
		const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack) {
	int rv = 0;
	FILE *file = NULL;
	struct SqshFileContext *content;
	uint16_t mode = sqsh_inode_permission(inode);

	content = sqsh_file_new(inode, &rv);
	if (rv < 0) {
		print_err(rv, "sqsh_file_new", path_stack);
		goto out;
	}

	rv = sqsh_file_read(content, sqsh_inode_file_size(inode));
	if (rv < 0) {
		print_err(rv, "sqsh_file_read", path_stack);
		goto out;
	}

	file = fopen(filename, "w");
	if (file == NULL) {
		print_err(rv = -errno, "fopen", path_stack);
		goto out;
	}
	fwrite(sqsh_file_data(content), sizeof(uint8_t), sqsh_file_size(content),
		   file);
	fclose(file);
	rv = chmod(filename, mode);
	if (rv < 0) {
		print_err(rv = -errno, "chmod", path_stack);
		goto out;
	}
out:
	sqsh_file_free(content);
	return rv;
}

static int
extract_symlink(
		const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack) {
	int rv;
	const char *target_ptr = sqsh_inode_symlink(inode);
	size_t size = sqsh_inode_symlink_size(inode);
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
		const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack) {
	int rv = 0;
	uint16_t mode = sqsh_inode_permission(inode);
	switch (sqsh_inode_type(inode)) {
	case SQSH_INODE_TYPE_BLOCK:
		mode |= S_IFCHR;
		break;
	case SQSH_INODE_TYPE_CHAR:
		mode |= S_IFBLK;
		break;
	case SQSH_INODE_TYPE_FIFO:
		mode |= S_IFIFO;
		break;
	case SQSH_INODE_TYPE_SOCKET:
		mode |= S_IFSOCK;
		break;
	default:
		print_err(-EINVAL, "extract_device", path_stack);
		rv = -EINVAL;
		goto out;
	}

	rv = mknod(filename, mode, sqsh_inode_device_id(inode));
	if (rv < 0) {
		print_err(rv = -errno, "mknod", path_stack);
		goto out;
	}
out:
	return rv;
}

static int
extract(const char *filename, struct SqshInodeContext *inode,
		const struct PathStack *path_stack) {
	const struct PathStack new_path_stack = {
			.segment = filename, .prev = path_stack};
	int rv = 0;
	uint32_t fuid, fgid;
	enum SqshInodeContextType type = sqsh_inode_type(inode);

	if (verbose) {
		print_path(&new_path_stack, "\n", stderr);
	}

	if (type == SQSH_INODE_TYPE_DIRECTORY) {
		rv = extract_directory(filename, inode, &new_path_stack);
	} else {
		rv = unlink(filename);
		if (rv < 0 && errno != ENOENT) {
			print_err(rv = -errno, "unlink", path_stack);
			goto out;
		}

		switch (sqsh_inode_type(inode)) {
		case SQSH_INODE_TYPE_FILE:
			rv = extract_file(filename, inode, &new_path_stack);
			break;
		case SQSH_INODE_TYPE_SYMLINK:
			rv = extract_symlink(filename, inode, &new_path_stack);
			break;
		case SQSH_INODE_TYPE_BLOCK:
		case SQSH_INODE_TYPE_CHAR:
		case SQSH_INODE_TYPE_FIFO:
		case SQSH_INODE_TYPE_SOCKET:
			rv = extract_device(filename, inode, &new_path_stack);
			break;
		default:
			print_err(-EINVAL, "extract", &new_path_stack);
			rv = -EINVAL;
		}
	}
	if (rv < 0) {
		goto out;
	}

	if (do_chown) {
		fuid = sqsh_inode_uid(inode);
		fgid = sqsh_inode_gid(inode);
		rv = chown(filename, fuid, fgid);
		if (rv < 0) {
			print_err(rv = -errno, "chown", &new_path_stack);
			goto out;
		}
	}

out:
	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *image_path;
	char *src_path = "/";
	char *target_path = NULL;
	struct SqshArchive *sqsh;
	struct SqshPathResolver *resolver = NULL;
	struct SqshInodeContext *inode = NULL;

	while ((opt = getopt(argc, argv, "cvVh")) != -1) {
		switch (opt) {
		case 'v':
			puts("sqsh-unpack-" VERSION);
			return 0;
		case 'V':
			verbose = true;
			break;
		case 'c':
			do_chown = true;
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

	sqsh = open_archive(image_path, &rv);
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

	inode = sqsh_path_resolver_resolve(resolver, src_path, &rv);
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
	} else if (sqsh_inode_type(inode) != SQSH_INODE_TYPE_DIRECTORY) {
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

	rv = extract(target_path, inode, NULL);
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}
out:
	sqsh_inode_free(inode);
	sqsh_path_resolver_free(resolver);
	sqsh_archive_free(sqsh);
	return rv;
}
