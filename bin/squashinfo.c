/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Apr 30, 2021 13:00:20 CEST
 */

#define _XOPEN_SOURCE

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "printb.h"

#include "../src/compression/compression.h"
#include "../src/directory.h"
#include "../src/format/compression_options.h"
#include "../src/format/metablock.h"
#include "../src/inode.h"
#include "../src/squash.h"

#define KEY_LENGTH "25"

#define KEY(x) fprintf(out, "%-" KEY_LENGTH "s ", x ":")

static FILE *out;

static int
usage(char *arg0) {
	printf("usage: %s FILESYSTEM [PATH]\n", arg0);
	return EXIT_FAILURE;
}

static int
metablock_info(const struct SquashMetablock *metablock, struct Squash *squash) {
	struct SquashExtract extract = {0};
	KEY("METABLOCK_INFO");
	if (metablock == NULL) {
		fputs("(none)\n", out);
		return 0;
	}

	int is_compressed = squash_metablock_is_compressed(metablock);
	squash_extract_init(&extract, squash, metablock, 0, 0);
	squash_extract_more(&extract, squash_metablock_size(metablock));
	size_t size = squash_extract_size(&extract);
	squash_extract_cleanup(&extract);

	size_t compressed_size = squash_metablock_size(metablock);
	fprintf(out,
			"compressed: %s, "
			"size: %lu, "
			"compressed_size: %lu, "
			"ratio: %f\n",
			is_compressed ? "yes" : "no", size, compressed_size,
			(double)compressed_size / size);
	return 0;
}

static int
compression_info_gzip(struct Squash *squash) {
	const struct SquashCompressionOptionsGzip *options =
			&squash->compression.options->gzip;

	KEY("COMPRESSION_LEVEL");
	fprintf(out, "%i\n", options->compression_level);
	KEY("WINDOW_SIZE");
	fprintf(out, "%i\n", options->window_size);
	KEY("STRATEGIES");
	printb(options->strategies, out);
	return 0;
}

static int
inode_info(struct Squash *squash, struct SquashInode *inode) {
	int rv = 0;

	KEY("HARDLINK COUNT");
	fprintf(out, "%u\n", squash_inode_hard_link_count(inode));

	return rv;
}

static int
dir_info(struct Squash *squash, struct SquashDirectory *dir) {
	int rv = 0;
	struct SquashDirectoryIterator iter = {0};
	const struct SquashDirectoryEntry *entry;
	fputs("=== ROOT DIRECTORY ===\n", out);

	squash_directory_iterator_init(&iter, dir);
	while ((entry = squash_directory_iterator_next(&iter))) {
		char *name = NULL;
		squash_directory_entry_name(entry, &name);
		KEY("FILE");
		fputs(name, out);
		fputc('\n', out);
		free(name);
	}
	squash_directory_iterator_clean(&iter);

	return rv;
}

static int
root_inode_info(struct Squash *squash) {
	struct SquashInode inode = {0};
	struct SquashDirectory dir = {0};
	int rv = 0;
	uint64_t root_inode_ref =
			squash_superblock_root_inode_ref(squash->superblock);
	fputs("=== INODE TABLE ===\n", out);
	rv = metablock_info(
			squash_metablock_from_offset(squash,
					squash_superblock_inode_table_start(squash->superblock)),
			squash);
	if (rv < 0) {
		return rv;
	}

	fputs("=== ROOT INODE ===\n", out);
	KEY("ROOT_INODE_ADDRESS");
	fprintf(out, "0x%lx\n", root_inode_ref);

	KEY("ROOT_INODE_REF");
	fprintf(out, "0x%lx\n", root_inode_ref);
	rv = squash_inode_load_ref(&inode, squash, root_inode_ref);
	if (rv < 0) {
		return rv;
	}
	inode_info(squash, &inode);
	rv = squash_directory_init(&dir, squash, &inode);
	if (rv < 0) {
		return rv;
	}

	dir_info(squash, &dir);

	squash_directory_cleanup(&dir);

	rv = squash_inode_cleanup(&inode);
	return rv;
}

static int
compression_info(struct Squash *squash) {
	int (*handler)(struct Squash * squash) = NULL;
	fputs("=== COMPRESSION INFO ===\n", out);
	KEY("COMPRESSION_TYPE");
	enum SquashSuperblockCompressionId id =
			squash_superblock_compression_id(squash->superblock);
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

	if (handler == NULL) {
		fputs("WARNING: NO COMPRESSION OPTION HANDLER\n", stdout);
	} else if (squash_superblock_flags(squash->superblock) &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		metablock_info(
				squash_metablock_from_offset(squash, SQUASH_SUPERBLOCK_SIZE),
				squash);
		handler(squash);
	} else {
		fputs("NO COMPRESSION OPTION\n", stdout);
	}
	return 0;
}

static int
flag_info(struct Squash *squash) {
	enum SquashSuperblockFlags flags =
			squash_superblock_flags(squash->superblock);
	fputs("=== FLAG INFO ===\n", out);
	KEY("FLAGS");
	printb(flags, out);
#define PRINT_FLAG(x) \
	{ \
		KEY(#x); \
		fputc(flags &SQUASH_SUPERBLOCK_##x ? '1' : '0', out); \
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

	root_inode_info(&squash);

	squash_cleanup(&squash);

	return 0;
}
