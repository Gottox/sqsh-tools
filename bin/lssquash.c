/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Sep 04, 2021 18:46:20 CEST
 */

#define _XOPEN_SOURCE

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/squash.h"

static int
usage(char *arg0) {
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

int
main(int argc, char *argv[]) {
	int rv;
	int opt = 0;
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	struct Squash squash = {0};
	const struct SquashDirectoryEntry *entry;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		default:
			return usage(argv[0]);
		}
	}

	if (optind + 1 != argc) {
		return usage(argv[0]);
	}

	rv = squash_open(&squash, argv[optind]);
	if (rv < 0) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}

	rv = squash_inode_load_ref(&inode, &squash,
			squash_superblock_root_inode_ref(squash.superblock));
	if (rv < 0) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}
	rv = squash_directory_init(&dir, &squash, &inode);
	if (rv < 0) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}

	squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}

	while ((entry = squash_directory_iterator_next(&iter))) {
		char *name;
		squash_directory_entry_name(entry, &name);
		printf("%s\n", name);
		free(name);
	}

	squash_directory_iterator_clean(&iter);
	squash_directory_cleanup(&dir);
	squash_inode_cleanup(&inode);

	squash_cleanup(&squash);

	return 0;
}
