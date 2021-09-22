/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lssquash
 * @created     : Friday Apr 30, 2021 13:00:20 CEST
 */

#include "../src/compression/compression.h"
#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/metablock_context.h"
#include "../src/data/compression_options_internal.h"
#include "../src/data/metablock.h"
#include "../src/data/superblock_internal.h"
#include "../src/squash.h"

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "printb.h"

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
	int rv = 0;
	struct SquashMetablockContext extract = {0};
	KEY("METABLOCK_INFO");
	if (metablock == NULL) {
		fputs("(none)\n", out);
		return 0;
	}

	int is_compressed = squash_data_metablock_is_compressed(metablock);
	rv = squash_metablock_init(&extract, squash->superblock,
			(uint8_t *)metablock - (uint8_t *)squash->superblock);
	if (rv < 0) {
		return 0;
	}
	rv = squash_metablock_more(&extract, squash_data_metablock_size(metablock));
	if (rv < 0) {
		squash_metablock_cleanup(&extract);
		return 0;
	}
	size_t size = squash_metablock_size(&extract);
	squash_metablock_cleanup(&extract);

	size_t compressed_size = squash_data_metablock_size(metablock);
	fprintf(out,
			"compressed: %s, "
			"size: %lu, "
			"compressed_size: %lu, "
			"ratio: %f\n",
			is_compressed ? "yes" : "no", size, compressed_size,
			(double)compressed_size / size);
	return rv;
}

static int
compression_info_gzip(struct Squash *squash) {
	int rv = 0;
	// TODO: use metablock context instead of compression here. this block is
	// never compressed.
	struct SquashBuffer buffer = {0};
	rv = squash_buffer_init(&buffer, squash->superblock, 8192);
	const struct SquashCompressionOptionsGzip *options = &buffer.options->gzip;

	KEY("COMPRESSION_LEVEL");
	fprintf(out, "%i\n", options->compression_level);
	KEY("WINDOW_SIZE");
	fprintf(out, "%i\n", options->window_size);
	KEY("STRATEGIES");
	printb(options->strategies, out);

	rv = squash_buffer_cleanup(&buffer);
	return rv;
}

static int
inode_info(struct Squash *squash, struct SquashInodeContext *inode) {
	int rv = 0;

	KEY("HARDLINK COUNT");
	fprintf(out, "%u\n", squash_inode_hard_link_count(inode));

	return rv;
}

static int
dir_info(struct Squash *squash, struct SquashDirectoryContext *dir) {
	int rv = 0;
	struct SquashDirectoryIterator iter = {0};
	const struct SquashDirectoryEntry *entry;
	fputs("=== ROOT DIRECTORY ===\n", out);

	rv = squash_directory_iterator_init(&iter, dir);
	if (rv < 0) {
		return 0;
	}
	while ((entry = squash_directory_iterator_next(&iter))) {
		char *name = NULL;
		rv = squash_directory_entry_name_dup(entry, &name);
		if (rv < 0) {
			break;
		}
		KEY("FILE");
		fputs(name, out);
		fputc('\n', out);
		free(name);
	}
	squash_directory_iterator_cleanup(&iter);

	return rv;
}

static int
root_inode_info(struct Squash *squash) {
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	int rv = 0;
	uint64_t root_inode_ref =
			squash_data_superblock_root_inode_ref(squash->superblock);
	fputs("=== INODE TABLE ===\n", out);
	rv = metablock_info(squash_metablock_from_offset(squash->superblock,
								squash_data_superblock_inode_table_start(
										squash->superblock)),
			squash);
	if (rv < 0) {
		return rv;
	}

	fputs("=== ROOT INODE ===\n", out);
	KEY("ROOT_INODE_ADDRESS");
	fprintf(out, "0x%lx\n", root_inode_ref);

	KEY("ROOT_INODE_REF");
	fprintf(out, "0x%lx\n", root_inode_ref);
	rv = squash_inode_load(&inode, squash->superblock, root_inode_ref);
	if (rv < 0) {
		return rv;
	}
	inode_info(squash, &inode);
	rv = squash_directory_init(&dir, squash->superblock, &inode);
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
			squash_data_superblock_compression_id(squash->superblock);
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
	} else if (squash_data_superblock_flags(squash->superblock) &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		metablock_info(squash_metablock_from_offset(squash->superblock,
							   sizeof(struct SquashSuperblock)),
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
			squash_data_superblock_flags(squash->superblock);
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

static int
section_info(struct Squash *squash) {
	const struct SquashSuperblock *s = squash->superblock;
	KEY("ID TABLE");
	fprintf(out, "%" PRId64 "\n", squash_data_superblock_id_table_start(s));
	KEY("XATTR TABLE");
	fprintf(out, "%" PRId64 "\n",
			squash_data_superblock_xattr_id_table_start(s));
	KEY("INODE TABLE");
	fprintf(out, "%" PRId64 "\n", squash_data_superblock_inode_table_start(s));
	KEY("DIR. TABLE");
	fprintf(out, "%" PRId64 "\n",
			squash_data_superblock_directory_table_start(s));
	KEY("FRAG TABLE");
	fprintf(out, "%" PRId64 "\n",
			squash_data_superblock_fragment_table_start(s));
	KEY("FRAGMENT ENTRIES");
	fprintf(out, "%u\n", squash_data_superblock_fragment_entry_count(s));
	KEY("EXPORT TABLE");
	fprintf(out, "%" PRId64 "\n", squash_data_superblock_export_table_start(s));
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
	section_info(&squash);
	compression_info(&squash);

	root_inode_info(&squash);

	squash_cleanup(&squash);

	return 0;
}
