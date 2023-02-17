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
 * @file         sqsh-ls.c
 */

#include "common.h"

#include <sqsh_context.h>
#include <sqsh_directory.h>

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
ls(struct SqshPathResolverContext *resolver, const char *path,
   struct SqshInodeContext *inode);

static int
usage(char *arg0) {
	printf("usage: %s [-r] [-l] FILESYSTEM [PATH]\n", arg0);
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
print_detail_inode(struct SqshInodeContext *inode, const char *path) {
	int mode;
	char xchar, unxchar;

	switch (sqsh_inode_type(inode)) {
	case SQSH_INODE_TYPE_UNKNOWN:
		putchar('?');
		break;
	case SQSH_INODE_TYPE_DIRECTORY:
		putchar('d');
		break;
	case SQSH_INODE_TYPE_FILE:
		putchar('-');
		break;
	case SQSH_INODE_TYPE_SYMLINK:
		putchar('l');
		break;
	case SQSH_INODE_TYPE_BLOCK:
		putchar('b');
		break;
	case SQSH_INODE_TYPE_CHAR:
		putchar('c');
		break;
	case SQSH_INODE_TYPE_FIFO:
		putchar('p');
		break;
	case SQSH_INODE_TYPE_SOCKET:
		putchar('s');
		break;
	}

	mode = sqsh_inode_permission(inode);
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

	time_t mtime = sqsh_inode_modified_time(inode);
	printf(" %6u %6u %10" PRIu64 " %s %s", sqsh_inode_uid(inode),
		   sqsh_inode_gid(inode), sqsh_inode_file_size(inode),
		   strtok(ctime(&mtime), "\n"), path);

	if (sqsh_inode_type(inode) == SQSH_INODE_TYPE_SYMLINK) {
		fputs(" -> ", stdout);
		fwrite(sqsh_inode_symlink(inode), sqsh_inode_symlink_size(inode),
			   sizeof(char), stdout);
	}

	putchar('\n');
}

static int
print_detail(const struct SqshDirectoryIterator *iter, const char *path) {
	int rv = 0;
	struct SqshInodeContext *inode = NULL;

	inode = sqsh_directory_iterator_inode_load(iter, &rv);
	if (rv < 0) {
		goto out;
	}
	print_detail_inode(inode, path);
out:
	sqsh_inode_free(inode);
	return rv;
}

static int
ls_item(struct SqshPathResolverContext *resolver, const char *path,
		struct SqshDirectoryIterator *iter) {
	int rv = 0;
	int len = 0;
	struct SqshInodeContext *entry_inode = NULL;
	const char *name = sqsh_directory_iterator_name(iter);
	const int name_size = sqsh_directory_iterator_name_size(iter);
	char *current_path =
			calloc(name_size + strlen(path ? path : "") + 2, sizeof(char));

	if (current_path == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	if (path != NULL) {
		strcpy(current_path, path);
		len = strlen(path);
		if (len > 0 && path[len - 1] != '/') {
			strcat(current_path, "/");
		}
	}
	strncat(current_path, name, name_size);
	print_item(iter, current_path);

	if (recursive &&
		sqsh_directory_iterator_inode_type(iter) == SQSH_INODE_TYPE_DIRECTORY) {
		entry_inode = sqsh_directory_iterator_inode_load(iter, &rv);
		if (rv < 0) {
			goto out;
		}
		rv = ls(resolver, current_path, entry_inode);
		if (rv < 0) {
			goto out;
		}
		sqsh_inode_free(entry_inode);
	}

out:
	free(current_path);
	return rv;
}

static int
ls(struct SqshPathResolverContext *resolver, const char *path,
   struct SqshInodeContext *inode) {
	int rv = 0;
	struct SqshDirectoryIterator *iter = NULL;

	iter = sqsh_directory_iterator_new(inode, &rv);
	if (rv < 0) {
		sqsh_perror(rv, "sqsh_directory_iterator_new");
		rv = EXIT_FAILURE;
		goto out;
	}

	while (sqsh_directory_iterator_next(iter) > 0) {
		rv = ls_item(resolver, path, iter);
		if (rv < 0) {
			rv = EXIT_FAILURE;
			goto out;
		}
	}

out:
	sqsh_directory_iterator_free(iter);

	return rv;
}

static int
ls_path(struct SqshPathResolverContext *resolver, char *path) {
	struct SqshInodeContext *inode = NULL;
	int rv = 0;

	inode = sqsh_path_resolver_resolve(resolver, path, &rv);
	if (rv < 0) {
		sqsh_perror(rv, path);
		goto out;
	}
	if (sqsh_inode_type(inode) == SQSH_INODE_TYPE_DIRECTORY) {
		if (rv < 0) {
			sqsh_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}

		rv = ls(resolver, path, inode);
		if (rv < 0) {
			sqsh_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}
	} else {
		if (print_item == print_detail) {
			print_detail_inode(inode, path);
		} else {
			puts(path);
		}
	}
out:
	sqsh_inode_free(inode);
	return rv;
}

int
main(int argc, char *argv[]) {
	bool has_listed = false;
	int rv = 0;
	int opt = 0;
	const char *image_path;
	struct Sqsh *sqsh;
	struct SqshPathResolverContext *resolver = NULL;

	while ((opt = getopt(argc, argv, "vrhl")) != -1) {
		switch (opt) {
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

	for (; optind < argc; optind++) {
		has_listed = true;
		rv = ls_path(resolver, argv[optind]);
		if (rv < 0) {
			goto out;
		}
	}

	if (has_listed == false) {
		rv = ls_path(resolver, NULL);
		if (rv < 0) {
			goto out;
		}
	}

out:
	sqsh_path_resolver_free(resolver);
	sqsh_free(sqsh);
	return rv;
}
