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
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int
print_simple(const char *path, const struct SqshTreeTraversal *traversal);

static bool recursive = false;
static bool utc = false;
static int (*print_item)(const char *, const struct SqshTreeTraversal *) =
		print_simple;

static int
usage(char *arg0) {
	printf("usage: %s [-o OFFSET] [-r] [-l] [-u] FILESYSTEM [PATH]\n", arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static void
print_mode(
		int mode, int r_mask, int w_mask, int x_mask, int s_mask,
		const char *s_chars) {
	const char *x_chars = (mode & s_mask) ? s_chars : "-x";

	putchar((mode & r_mask) ? 'r' : '-');
	putchar((mode & w_mask) ? 'w' : '-');
	putchar((mode & x_mask) ? x_chars[1] : x_chars[0]);
}

static void
print_segment(const char *segment, size_t segment_size) {
	for (size_t i = 0; i < segment_size; i++) {
		switch (segment[i]) {
		case '\n':
			fputs("\\n", stdout);
			break;
		case '\r':
			fputs("\\r", stdout);
			break;
		case '\t':
			fputs("\\t", stdout);
			break;
		case '\\':
			fputs("\\\\", stdout);
			break;
		case '\x1b':
			fputs("\\e", stdout);
			break;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x0b:
		case 0x0c:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
		case 0x20:
		case 0x7f:
			printf("\\x%02x", segment[i]);
			break;
		default:
			putchar(segment[i]);
			break;
		}
	}
}

static void
print_path(const char *path, const struct SqshTreeTraversal *traversal) {
	fputs(path, stdout);
	size_t segment_count = sqsh_tree_traversal_depth(traversal);
	const char *segment;
	size_t segment_size;
	for (size_t i = 0; i < segment_count; i++) {
		segment = sqsh_tree_traversal_path_segment(traversal, &segment_size, i);
		putchar('/');
		print_segment(segment, segment_size);
	}
}

static int
print_simple(const char *path, const struct SqshTreeTraversal *traversal) {
	print_path(path, traversal);
	putchar('\n');
	return 0;
}

static int
print_detail(const char *path, const struct SqshTreeTraversal *traversal) {
	int mode;
	int rv = 0;

	struct SqshFile *file = sqsh_tree_traversal_open_file(traversal, &rv);
	if (rv < 0) {
		goto out;
	}

	time_t mtime = sqsh_file_modified_time(file);
	struct tm tm_info_buf = {0};
	const struct tm *tm_info;
	char time_buffer[128] = {0};
	tm_info = utc ? gmtime_r(&mtime, &tm_info_buf)
				  : localtime_r(&mtime, &tm_info_buf);
	if (tm_info == NULL) {
		return -errno;
	}

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
	print_mode(mode, S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID, "Ss");
	print_mode(mode, S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID, "Ss");
	print_mode(mode, S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX, "Tt");

	strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %T %z", tm_info);

	printf(" %6u %6u %10" PRIu64 " %s ", sqsh_file_uid(file),
		   sqsh_file_gid(file), sqsh_file_size(file), time_buffer);

	print_path(path, traversal);

	if (sqsh_file_type(file) == SQSH_FILE_TYPE_SYMLINK) {
		fputs(" -> ", stdout);
		fwrite(sqsh_file_symlink(file), sqsh_file_symlink_size(file),
			   sizeof(char), stdout);
	}

	putchar('\n');

out:
	sqsh_close(file);
	return rv;
}

static int
ls_path(struct SqshArchive *archive, char *path) {
	int rv = 0;
	struct SqshFile *file;
	size_t path_size = strlen(path);
	struct SqshTreeTraversal *traversal = NULL;

	while (path_size > 0 && path[path_size - 1] == '/') {
		path_size--;
		path[path_size] = '\0';
	}

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	traversal = sqsh_tree_traversal_new2(file, recursive ? 0 : 1, &rv);
	if (rv < 0) {
		goto out;
	}

	while (sqsh_tree_traversal_next(traversal, &rv)) {
		if (rv < 0) {
			goto out;
		}
		enum SqshTreeTraversalState state =
				sqsh_tree_traversal_state(traversal);
		size_t depth = sqsh_tree_traversal_depth(traversal);
		if (depth == 0 && state == SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN) {
			continue;
		} else if (state == SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END) {
			continue;
		}

		rv = print_item(path, traversal);
		if (rv < 0) {
			goto out;
		}
	}
out:
	if (rv < 0) {
		sqsh_perror(rv, path);
		rv = 0;
	}
	sqsh_tree_traversal_free(traversal);
	sqsh_close(file);
	return rv;
}

static const char opts[] = "o:vrhlu";
static const struct option long_opts[] = {
		{"offset", required_argument, NULL, 'o'},
		{"version", no_argument, NULL, 'v'},
		{"recursive", no_argument, NULL, 'r'},
		{"help", no_argument, NULL, 'h'},
		{"long", no_argument, NULL, 'l'},
		{"utc", no_argument, NULL, 'u'},
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
		case 'u':
			utc = true;
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

	// Having TZ be set to its default value prevents libc to repeatedly
	// stat /etc/localtime for every localtime() call.
	setenv("TZ", ":/etc/localtime", 0);
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
		rv = ls_path(archive, "");
		if (rv < 0) {
			goto out;
		}
	}

out:
	if (rv < 0) {
		rv = EXIT_FAILURE;
	}
	sqsh_archive_close(archive);
	return rv;
}
