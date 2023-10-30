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
 * @file         ls.c
 */

#include <sqshtools_common.h>

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int
print_simple(const struct SqshDirectoryIterator *iter, const char *path);

static bool recursive = false;
static int (*print_item)(const struct SqshDirectoryIterator *, const char *) =
		print_simple;

static int
ls(struct SqshArchive *archive, const char *path, struct SqshFile *file);

static int
usage(char *arg0) {
	printf("usage: %s [-o OFFSET] [-r] [-l] FILESYSTEM [PATH]\n", arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static int
print_simple(const struct SqshDirectoryIterator *iter, const char *path) {
	(void)iter;
	puts(path);
	return 0;
}

void
print_detail_file(struct SqshFile *file, const char *path) {
	int mode;
	char xchar, unxchar;

	switch (sqsh_file_type(file)) {
	case SQSH_FILE_TYPE_UNKNOWN:
		putchar('?');
		break;
	case SQSH_FILE_TYPE_DIRECTORY:
		putchar('d');
		break;
	case SQSH_FILE_TYPE_FILE:
		putchar('-');
		break;
	case SQSH_FILE_TYPE_SYMLINK:
		putchar('l');
		break;
	case SQSH_FILE_TYPE_BLOCK:
		putchar('b');
		break;
	case SQSH_FILE_TYPE_CHAR:
		putchar('c');
		break;
	case SQSH_FILE_TYPE_FIFO:
		putchar('p');
		break;
	case SQSH_FILE_TYPE_SOCKET:
		putchar('s');
		break;
	}

	mode = sqsh_file_permission(file);
#define PRINT_MODE(t) \
	{ \
		putchar((S_IR##t & mode) ? 'r' : '-'); \
		putchar((S_IW##t & mode) ? 'w' : '-'); \
		putchar((S_IX##t & mode) ? xchar : unxchar); \
	}
	// TODO: these macros depend on system specific values
	xchar = (S_ISUID & mode) ? 's' : 'x';
	unxchar = (S_ISUID & mode) ? 'S' : '-';
	PRINT_MODE(USR);
	xchar = (S_ISGID & mode) ? 's' : 'x';
	unxchar = (S_ISGID & mode) ? 'S' : '-';
	PRINT_MODE(GRP);
	xchar = (S_ISVTX & mode) ? 't' : 'x';
	unxchar = (S_ISVTX & mode) ? 'T' : '-';
	PRINT_MODE(OTH);
#undef PRINT_MODE

	time_t mtime = sqsh_file_modified_time(file);
	printf(" %6u %6u %10" PRIu64 " %s %s", sqsh_file_uid(file),
		   sqsh_file_gid(file), sqsh_file_size(file),
		   strtok(ctime(&mtime), "\n"), path);

	if (sqsh_file_type(file) == SQSH_FILE_TYPE_SYMLINK) {
		fputs(" -> ", stdout);
		fwrite(sqsh_file_symlink(file), sqsh_file_symlink_size(file),
			   sizeof(char), stdout);
	}

	putchar('\n');
}

static int
print_detail(const struct SqshDirectoryIterator *iter, const char *path) {
	int rv = 0;
	struct SqshFile *file = NULL;

	file = sqsh_directory_iterator_open_file(iter, &rv);
	if (rv < 0) {
		goto out;
	}
	print_detail_file(file, path);
out:
	sqsh_close(file);
	return rv;
}

static int
ls_item(struct SqshArchive *archive, const char *path,
		struct SqshDirectoryIterator *iter) {
	int rv = 0;
	struct SqshFile *entry = NULL;
	const char *name = sqsh_directory_iterator_name(iter);
	const int name_size = sqsh_directory_iterator_name_size(iter);
	const size_t path_len = path ? strlen(path) : 0;
	const size_t current_path_size = name_size + path_len + 2;
	char *current_path = calloc(current_path_size, sizeof(char));

	if (current_path == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	if (path != NULL) {
		memcpy(current_path, path, path_len);
		if (path_len > 0 && path[path_len - 1] != '/') {
			strncat(current_path, "/", 2);
		}
	}
	strncat(current_path, name, name_size);
	print_item(iter, current_path);

	if (recursive &&
		sqsh_directory_iterator_file_type(iter) == SQSH_FILE_TYPE_DIRECTORY) {
		entry = sqsh_directory_iterator_open_file(iter, &rv);
		if (rv < 0) {
			goto out;
		}
		rv = ls(archive, current_path, entry);
		if (rv < 0) {
			goto out;
		}
		sqsh_close(entry);
	}

out:
	free(current_path);
	return rv;
}

static int
ls(struct SqshArchive *archive, const char *path, struct SqshFile *file) {
	int rv = 0;
	struct SqshDirectoryIterator *iter = NULL;

	iter = sqsh_directory_iterator_new(file, &rv);
	if (rv < 0) {
		sqsh_perror(rv, "sqsh_directory_iterator_new");
		rv = EXIT_FAILURE;
		goto out;
	}

	while (sqsh_directory_iterator_next(iter, &rv)) {
		rv = ls_item(archive, path, iter);
		if (rv < 0) {
			break;
		}
	}
	if (rv < 0) {
		rv = EXIT_FAILURE;
		goto out;
	}

out:
	sqsh_directory_iterator_free(iter);

	return rv;
}

static int
ls_path(struct SqshArchive *archive, char *path) {
	struct SqshFile *file = NULL;
	int rv = 0;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		sqsh_perror(rv, path);
		goto out;
	}
	if (sqsh_file_type(file) == SQSH_FILE_TYPE_DIRECTORY) {
		if (rv < 0) {
			sqsh_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}

		rv = ls(archive, path, file);
		if (rv < 0) {
			sqsh_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}
	} else {
		if (print_item == print_detail) {
			print_detail_file(file, path);
		} else {
			puts(path);
		}
	}
out:
	sqsh_close(file);
	return rv;
}

static const char opts[] = "o:vrhl";
static const struct option long_opts[] = {
		{"offset", required_argument, NULL, 'o'},
		{"version", no_argument, NULL, 'v'},
		{"recursive", no_argument, NULL, 'r'},
		{"help", no_argument, NULL, 'h'},
		{"long", no_argument, NULL, 'l'},
		{0},
};

int
main(int argc, char *argv[]) {
	bool has_listed = false;
	int rv = 0;
	int opt = 0;
	const char *image_path;
	struct SqshArchive *archive;
	uint64_t offset = 0;

	while ((opt = getopt_long(argc, argv, opts, long_opts, NULL)) != -1) {
		switch (opt) {
		case 'o':
			offset = strtoull(optarg, NULL, 0);
			break;
		case 'v':
			puts("sqsh-ls-" VERSION);
			return 0;
		case 'r':
			recursive = true;
			break;
		case 'l':
			print_item = print_detail;
			break;
		default:
			return usage(argv[0]);
		}
	}

	if (optind >= argc) {
		return usage(argv[0]);
	}

	image_path = argv[optind];
	optind++;

	archive = open_archive(image_path, offset, &rv);
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	for (; optind < argc; optind++) {
		has_listed = true;
		rv = ls_path(archive, argv[optind]);
		if (rv < 0) {
			goto out;
		}
	}

	if (has_listed == false) {
		rv = ls_path(archive, "/");
		if (rv < 0) {
			goto out;
		}
	}

out:
	sqsh_archive_close(archive);
	return rv;
}
