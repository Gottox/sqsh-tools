/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Sep 04, 2021 18:46:20 CEST
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/data/superblock.h"
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
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *path = "";
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

	if (optind + 2 == argc) {
		path = argv[optind + 1];
	} else if (optind + 1 != argc) {
		return usage(argv[0]);
	}

	rv = squash_open(&squash, argv[optind]);
	if (rv < 0) {
		perror(argv[optind]);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = squash_resolve_path(&inode, squash.superblock, path);
	if (rv < 0) {
		perror(argv[optind]);
		rv = EXIT_FAILURE;
		goto out;
	}
	rv = squash_directory_init(&dir, squash.superblock, &inode);
	if (rv < 0) {
		perror(argv[optind]);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = squash_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		perror(argv[optind]);
		rv = EXIT_FAILURE;
		goto out;
	}

	while ((entry = squash_directory_iterator_next(&iter))) {
		const char *name = squash_directory_entry_name(entry);
		const int name_size = squash_directory_entry_name_size(entry);
		fwrite(name, sizeof(char), name_size, stdout);
		fputc('\n', stdout);
	}

out:
	squash_directory_iterator_cleanup(&iter);
	squash_directory_cleanup(&dir);
	squash_inode_cleanup(&inode);

	squash_cleanup(&squash);

	return rv;
}
