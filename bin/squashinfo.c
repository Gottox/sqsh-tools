/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Apr 30, 2021 13:00:20 CEST
 */

#include <stdio.h>
#include <unistd.h>

#include "../src/squash.h"

static int
usage(char *arg0) {
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

static int
compression_info(struct Squash *squash) {
	fputs("compression type: ", stdout);
	switch ((enum CompressionId)squash->superblock->compression_id) {
	case SQUASH_COMPRESSION_NONE:
		fputs("none\n", stdout);
		break;
	case SQUASH_COMPRESSION_GZIP:
		fputs("gzip\n", stdout);
		break;
	case SQUASH_COMPRESSION_LZMA:
		fputs("lzma\n", stdout);
		break;
	case SQUASH_COMPRESSION_LZO:
		fputs("lzo\n", stdout);
		break;
	case SQUASH_COMPRESSION_XZ:
		fputs("xz\n", stdout);
		break;
	case SQUASH_COMPRESSION_LZ4:
		fputs("lz4\n", stdout);
		break;
	case SQUASH_COMPRESSION_ZSTD:
		fputs("zstd\n", stdout);
		break;
	}

	fputs("comptession options: ", stdout);
	fputs((squash->superblock->flags & SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS)
			  ? "yes\n"
			  : "no\n",
		  stdout);

	return 0;
}

int
main(int argc, char *argv[]) {
	int opt = 0;
	const char *image_path;
	struct Squash *squash = NULL;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		default:
			return usage(argv[0]);
		}
	}

	if (optind + 2 == argc) {
		return usage(argv[0]);
	}

	squash = squash_open(argv[optind]);
	if (squash == NULL) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}

	compression_info(squash);

	squash_cleanup(squash);

	return 0;
}
