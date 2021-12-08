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
 * @file        : hsqs-ls
 * @created     : Friday Sep 04, 2021 18:46:20 CEST
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/hsqs.h"
#include "../src/resolve_path.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int
print_simple(const struct HsqsDirectoryIterator *iter, const char *path);

static bool recursive = false;
static int (*print_item)(const struct HsqsDirectoryIterator *, const char *) =
		print_simple;

static int
ls(struct Hsqs *hsqs, const char *path, struct HsqsInodeContext *inode);

static int
usage(char *arg0) {
	printf("usage: %s [-r] [-l] FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

static int
print_simple(const struct HsqsDirectoryIterator *iter, const char *path) {
	puts(path);
	return 0;
}

void
print_detail_inode(struct HsqsInodeContext *inode, const char *path) {
	int mode;
	char xchar, unxchar;

	switch (hsqs_inode_type(inode)) {
	case HSQS_INODE_TYPE_UNKNOWN:
		putchar('?');
		break;
	case HSQS_INODE_TYPE_DIRECTORY:
		putchar('d');
		break;
	case HSQS_INODE_TYPE_FILE:
		putchar('-');
		break;
	case HSQS_INODE_TYPE_SYMLINK:
		putchar('l');
		break;
	case HSQS_INODE_TYPE_BLOCK:
		putchar('b');
		break;
	case HSQS_INODE_TYPE_CHAR:
		putchar('c');
		break;
	case HSQS_INODE_TYPE_FIFO:
		putchar('p');
		break;
	case HSQS_INODE_TYPE_SOCKET:
		putchar('s');
		break;
	}

	mode = hsqs_inode_permission(inode);
#define PRINT_MODE(t) \
	{ \
		putchar((S_IR##t & mode) ? 'r' : '-'); \
		putchar((S_IW##t & mode) ? 'w' : '-'); \
		putchar((S_IX##t & mode) ? xchar : unxchar); \
	}
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

	time_t mtime = hsqs_inode_modified_time(inode);
	printf(" %6u %6u %10lu %s %s", hsqs_inode_uid(inode), hsqs_inode_gid(inode),
		   hsqs_inode_file_size(inode), strtok(ctime(&mtime), "\n"), path);

	if (hsqs_inode_type(inode) == HSQS_INODE_TYPE_SYMLINK) {
		fputs(" -> ", stdout);
		fwrite(hsqs_inode_symlink(inode), hsqs_inode_symlink_size(inode),
			   sizeof(char), stdout);
	}

	putchar('\n');
}

static int
print_detail(const struct HsqsDirectoryIterator *iter, const char *path) {
	int rv = 0;
	struct HsqsInodeContext inode = {0};

	rv = hsqs_directory_iterator_inode_load(iter, &inode);
	if (rv < 0) {
		goto out;
	}
	print_detail_inode(&inode, path);
out:
	hsqs_inode_cleanup(&inode);
	return rv;
}

static int
ls_item(struct Hsqs *hsqs, const char *path,
		struct HsqsDirectoryIterator *iter) {
	int rv = 0;
	int len = 0;
	struct HsqsInodeContext entry_inode = {0};
	const char *name = hsqs_directory_iterator_name(iter);
	const int name_size = hsqs_directory_iterator_name_size(iter);
	char *current_path =
			calloc(name_size + strlen(path ? path : "") + 2, sizeof(char));

	if (current_path == NULL) {
		rv = -HSQS_ERROR_MALLOC_FAILED;
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
		hsqs_directory_iterator_inode_type(iter) == HSQS_INODE_TYPE_DIRECTORY) {
		rv = hsqs_directory_iterator_inode_load(iter, &entry_inode);
		if (rv < 0) {
			goto out;
		}
		rv = ls(hsqs, current_path, &entry_inode);
		if (rv < 0) {
			goto out;
		}
		hsqs_inode_cleanup(&entry_inode);
	}

out:
	free(current_path);
	return rv;
}

static int
ls(struct Hsqs *hsqs, const char *path, struct HsqsInodeContext *inode) {
	int rv;
	struct HsqsDirectoryContext dir = {0};
	struct HsqsDirectoryIterator iter = {0};

	rv = hsqs_directory_init(&dir, &hsqs->superblock, inode);
	if (rv < 0) {
		hsqs_perror(rv, path == NULL || path[0] == 0 ? "/" : path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = hsqs_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		hsqs_perror(rv, "hsqs_directory_iterator_init");
		rv = EXIT_FAILURE;
		goto out;
	}

	while (hsqs_directory_iterator_next(&iter) > 0) {
		rv = ls_item(hsqs, path, &iter);
		if (rv < 0) {
			rv = EXIT_FAILURE;
			goto out;
		}
	}

out:
	hsqs_directory_cleanup(&dir);
	hsqs_directory_iterator_cleanup(&iter);

	return rv;
}

static int
ls_path(struct Hsqs *hsqs, char *path) {
	struct HsqsInodeContext inode = {0};
	int rv = 0;

	rv = hsqs_resolve_path(&inode, &hsqs->superblock, path);
	if (hsqs_inode_type(&inode) == HSQS_INODE_TYPE_DIRECTORY) {
		if (rv < 0) {
			hsqs_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}

		rv = ls(hsqs, path, &inode);
		if (rv < 0) {
			hsqs_perror(rv, path);
			rv = EXIT_FAILURE;
			goto out;
		}
	} else {
		if (print_item == print_detail) {
			print_detail_inode(&inode, path);
		} else {
			puts(path);
		}
	}
out:
	hsqs_inode_cleanup(&inode);
	return rv;
}

int
main(int argc, char *argv[]) {
	bool has_listed = false;
	int rv = 0;
	int opt = 0;
	const char *image_path;
	struct Hsqs hsqs = {0};

	while ((opt = getopt(argc, argv, "rhl")) != -1) {
		switch (opt) {
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

	rv = hsqs_open(&hsqs, image_path);
	if (rv < 0) {
		hsqs_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	for (; optind < argc; optind++) {
		has_listed = true;
		rv = ls_path(&hsqs, argv[optind]);
		if (rv < 0) {
			goto out;
		}
	}

	if (has_listed == false) {
		rv = ls_path(&hsqs, NULL);
		if (rv < 0) {
			goto out;
		}
	}

out:
	hsqs_cleanup(&hsqs);
	return rv;
}
