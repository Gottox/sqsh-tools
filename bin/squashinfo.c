/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Apr 30, 2021 13:00:20 CEST
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "printb.h"

#include "../src/compression/compression.h"
#include "../src/compression/gzip_handler.h"
#include "../src/metablock.h"
#include "../src/squash.h"
#include "../src/superblock.h"

#define KEY_LENGTH "25"

#define KEY(x) fprintf(out, "%-" KEY_LENGTH "s ", x ":")

static FILE *out;

static int
usage(char *arg0) {
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

static int
metablock_info(struct SquashMetablock *metablock) {
	KEY("METABLOCK_INFO");
	if (metablock) {
		int is_compressed = squash_metablock_is_compressed(metablock);
		size_t size = squash_metablock_compressed_size(metablock);
		size_t compressed_size = squash_metablock_compressed_size(metablock);
		fprintf(out, "compressed: %s, ", is_compressed ? "yes" : "no");
		fprintf(out, "size: %lu, ", size);
		fprintf(out, "compressed_size: %lu, ", compressed_size);
		fprintf(out, "ratio: %f\n", (double)size / compressed_size);
	} else {
		fputs("(none)\n", out);
	}
	return 0;
}

static int
compression_info_gzip(struct Squash *squash) {
	struct SquashGzip *options = &squash->decompressor.info.gzip;

	KEY("COMPRESSION_LEVEL");
	fprintf(out, "%i\n", options->options->compression_level);
	KEY("WINDOW_SIZE");
	fprintf(out, "%i\n", options->options->window_size);
	KEY("STRATEGIES");
	printb(options->options->strategies, out);
	return 0;
}

static int
compression_info(struct Squash *squash) {
	int (*handler)(struct Squash * squash) = NULL;
	fputs("=== COMPRESSION INFO ===\n", out);
	KEY("COMPRESSION_TYPE");
	enum SquashSuperblockCompressionId id = squash->superblock->compression_id;
	switch (id) {
	case SQUASH_COMPRESSION_NONE:
		fputs("none\n", out);
		break;
	case SQUASH_COMPRESSION_GZIP:
		fputs("gzip\n", out);
		handler = compression_info_gzip;
		break;
	case SQUASH_COMPRESSION_LZMA:
		fputs("lzma\n", out);
		break;
	case SQUASH_COMPRESSION_LZO:
		fputs("lzo\n", out);
		break;
	case SQUASH_COMPRESSION_XZ:
		fputs("xz\n", out);
		break;
	case SQUASH_COMPRESSION_LZ4:
		fputs("lz4\n", out);
		break;
	case SQUASH_COMPRESSION_ZSTD:
		fputs("zstd\n", out);
		break;
	}

	metablock_info(&squash->decompressor.metablock);
	if (handler == NULL) {
		fputs("WARNING: NO COMPRESSION OPTION HANDLER", stdout);
	} else if (squash->superblock->flags &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		handler(squash);
	}

	return 0;
}

static int
flag_info(struct Squash *squash) {
	fputs("=== FLAG INFO ===\n", out);
	KEY("FLAGS");
	printb(squash->superblock->flags, out);
#define PRINT_FLAG(x) \
	{ \
		KEY(#x); \
		fputc(squash->superblock->flags &SQUASH_SUPERBLOCK_##x ? '1' : '0', \
				out); \
		fputc('\n', out); \
	}
	PRINT_FLAG(UNCOMPRESSED_INODES);
	PRINT_FLAG(UNCOMPRESSED_DATA);
	PRINT_FLAG(CHECK);
	PRINT_FLAG(UNCOMPRESSED_FRAGMENTS);
	PRINT_FLAG(NO_FRAGMENTS);
	PRINT_FLAG(ALWAYS_FRAGMENTS);
	PRINT_FLAG(DUPLICATES);
	PRINT_FLAG(EXPORTABLE);
	PRINT_FLAG(UNCOMPRESSED_XATTRS);
	PRINT_FLAG(NO_XATTRS);
	PRINT_FLAG(COMPRESSOR_OPTIONS);
	PRINT_FLAG(UNCOMPRESSED_IDS);
#undef PRINT_FLAG

	return 0;
}

int
main(int argc, char *argv[]) {
	int rv;
	int opt = 0;
	const char *image_path;
	struct Squash squash = {0};

	out = stdout;

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

	flag_info(&squash);
	compression_info(&squash);

	squash_cleanup(&squash);

	return 0;
}
