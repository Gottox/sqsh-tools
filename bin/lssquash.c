/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Sep 04, 2021 18:46:20 CEST
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/resolve_path.h"
#include "../src/squash.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int
usage(char *arg0) {
	printf("usage: %s [-r] FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

static int
ls(struct Squash *squash, const char *path, struct SquashInodeContext *inode,
		const bool recursive) {
	int rv;
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	struct SquashInodeContext entry_inode = {0};

	rv = squash_directory_init(&dir, squash->superblock, inode);
	if (rv < 0) {
		squash_perror(rv, path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		squash_perror(rv, "squash_directory_iterator_init");
		rv = EXIT_FAILURE;
		goto out;
	}

	while (squash_directory_iterator_next(&iter) > 0) {
		const char *name = squash_directory_iterator_name(&iter);
		const int name_size = squash_directory_iterator_name_size(&iter);
		char *current_path =
				calloc(name_size + strlen(path ? path : "") + 2, sizeof(char));
		if (current_path == NULL) {
			rv = -SQUASH_ERROR_MALLOC_FAILED;
			goto out;
		}
		if (path != NULL) {
			strcpy(current_path, path);
			strcat(current_path, "/");
		}
		strncat(current_path, name, name_size);
		puts(current_path);
		if (recursive &&
				squash_directory_iterator_inode_type(&iter) ==
						SQUASH_INODE_TYPE_DIRECTORY) {
			rv = squash_directory_iterator_inode_load(&iter, &entry_inode);
			if (rv < 0) {
				goto out;
			}
			rv = ls(squash, current_path, &entry_inode, recursive);
			if (rv < 0) {
				goto out;
			}
			squash_inode_cleanup(&entry_inode);
		}
		free(current_path);
	}
	squash_directory_cleanup(&dir);

out:
	squash_directory_iterator_cleanup(&iter);

	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	bool recursive = false;
	const char *inner_path = "";
	const char *outer_path;
	struct SquashInodeContext inode = {0};
	struct Squash squash = {0};

	while ((opt = getopt(argc, argv, "rh")) != -1) {
		switch (opt) {
		case 'r':
			recursive = true;
			break;
		default:
			return usage(argv[0]);
		}
	}

	if (optind + 2 == argc) {
		inner_path = argv[optind + 1];
	} else if (optind + 1 != argc) {
		return usage(argv[0]);
	}
	outer_path = argv[optind];

	rv = squash_open(&squash, outer_path);
	if (rv < 0) {
		squash_perror(rv, outer_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = squash_resolve_path(&inode, squash.superblock, inner_path);
	if (rv < 0) {
		squash_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = ls(&squash, NULL, &inode, recursive);

out:
	squash_inode_cleanup(&inode);

	squash_cleanup(&squash);

	return rv;
}
