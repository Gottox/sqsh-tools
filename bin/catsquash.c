/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : catsquash
 * @created     : Monday Sep 20, 2021 09:48:58 CEST
 */

#include "../src/context/datablock_context.h"
#include "../src/context/directory_context.h"
#include "../src/context/fragment_context.h"
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
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *inner_path = "";
	const char *outer_path;
	struct SquashInodeContext inode = {0};
	struct SquashDatablockContext datablocks = {0};
	struct SquashFragmentContext fragment = {0};
	struct Squash squash = {0};

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
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

	rv = squash_datablock_init(&datablocks, squash.superblock, &inode);
	if (rv < 0) {
		squash_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = squash_datablock_read(&datablocks, squash_inode_file_size(&inode));
	if (rv < 0) {
		squash_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	fwrite(squash_datablock_data(&datablocks), sizeof(uint8_t),
			squash_datablock_size(&datablocks), stdout);

	rv = squash_fragment_init(&fragment, squash.superblock, &inode);
	if (rv < 0) {
		// We didn't check for the existence of a fragment. If none is there
		// just skip it.
		if (rv == -SQUASH_ERROR_NO_FRAGMENT) {
			rv = 0;
		}
		goto out;
	}

	fwrite(squash_fragment_data(&fragment), sizeof(uint8_t),
			squash_fragment_size(&fragment), stdout);

out:
	squash_datablock_clean(&datablocks);
	squash_inode_cleanup(&inode);

	squash_cleanup(&squash);

	return rv;
}
