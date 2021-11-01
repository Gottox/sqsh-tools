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
 * @file        : hsqs-cat
 * @created     : Friday Apr 30, 2021 13:00:20 CEST
 */

#include "../src/compression/compression.h"
#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/metablock_context.h"
#include "../src/data/compression_options_internal.h"
#include "../src/data/metablock.h"
#include "../src/data/superblock_internal.h"
#include "../src/hsqs.h"

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
metablock_info(const struct HsqsMetablock *metablock, struct Hsqs *hsqs) {
	int rv = 0;
	struct HsqsMetablockContext extract = {0};
	KEY("METABLOCK_INFO");
	if (metablock == NULL) {
		fputs("(none)\n", out);
		return 0;
	}

	int is_compressed = hsqs_data_metablock_is_compressed(metablock);
	rv = hsqs_metablock_init(
			&extract, &hsqs->superblock,
			(uint8_t *)metablock - (uint8_t *)hsqs->superblock.superblock);
	if (rv < 0) {
		return 0;
	}
	rv = hsqs_metablock_more(&extract, hsqs_data_metablock_size(metablock));
	if (rv < 0) {
		hsqs_metablock_cleanup(&extract);
		return 0;
	}
	size_t size = hsqs_metablock_size(&extract);
	hsqs_metablock_cleanup(&extract);

	size_t compressed_size = hsqs_data_metablock_size(metablock);
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
compression_info_gzip(struct Hsqs *hsqs) {
	int rv = 0;
	// TODO: use metablock context instead of compression here. this block is
	// never compressed.
	struct HsqsBuffer buffer = {0};
	rv = hsqs_buffer_init(&buffer, &hsqs->superblock, 8192);
	const struct HsqsCompressionOptionsGzip *options = &buffer.options->gzip;

	KEY("COMPRESSION_LEVEL");
	fprintf(out, "%i\n", options->compression_level);
	KEY("WINDOW_SIZE");
	fprintf(out, "%i\n", options->window_size);
	KEY("STRATEGIES");
	printb(options->strategies, out);

	rv = hsqs_buffer_cleanup(&buffer);
	return rv;
}

static int
inode_info(struct Hsqs *hsqs, struct HsqsInodeContext *inode) {
	int rv = 0;

	KEY("HARDLINK COUNT");
	fprintf(out, "%u\n", hsqs_inode_hard_link_count(inode));

	return rv;
}

static int
dir_info(struct Hsqs *hsqs, struct HsqsDirectoryContext *dir) {
	int rv = 0;
	struct HsqsDirectoryIterator iter = {0};
	fputs("=== ROOT DIRECTORY ===\n", out);

	rv = hsqs_directory_iterator_init(&iter, dir);
	if (rv < 0) {
		return 0;
	}
	while (hsqs_directory_iterator_next(&iter) > 0) {
		char *name = NULL;
		rv = hsqs_directory_iterator_name_dup(&iter, &name);
		if (rv < 0) {
			break;
		}
		KEY("FILE");
		fputs(name, out);
		fputc('\n', out);
		free(name);
	}
	hsqs_directory_iterator_cleanup(&iter);

	return rv;
}

static int
root_inode_info(struct Hsqs *hsqs) {
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryContext dir = {0};
	const struct HsqsMetablock *metablock = NULL;
	int rv = 0;
	uint64_t root_inode_ref =
			hsqs_data_superblock_root_inode_ref(hsqs->superblock.superblock);
	fputs("=== INODE TABLE ===\n", out);
	rv = hsqs_metablock_from_offset(
			&metablock, &hsqs->superblock,
			hsqs_data_superblock_inode_table_start(
					hsqs->superblock.superblock));
	if (rv < 0) {
		return rv;
	}
	rv = metablock_info(metablock, hsqs);
	if (rv < 0) {
		return rv;
	}

	fputs("=== ROOT INODE ===\n", out);
	KEY("ROOT_INODE_ADDRESS");
	fprintf(out, "0x%lx\n", root_inode_ref);

	KEY("ROOT_INODE_REF");
	fprintf(out, "0x%lx\n", root_inode_ref);
	rv = hsqs_inode_load(&inode, &hsqs->superblock, root_inode_ref);
	if (rv < 0) {
		return rv;
	}
	inode_info(hsqs, &inode);
	rv = hsqs_directory_init(&dir, &hsqs->superblock, &inode);
	if (rv < 0) {
		return rv;
	}

	dir_info(hsqs, &dir);

	hsqs_directory_cleanup(&dir);

	rv = hsqs_inode_cleanup(&inode);
	return rv;
}

static int
compression_info(struct Hsqs *hsqs) {
	int rv = 0;
	const struct HsqsMetablock *metablock = NULL;
	int (*handler)(struct Hsqs * hsqs) = NULL;
	fputs("=== COMPRESSION INFO ===\n", out);
	KEY("COMPRESSION_TYPE");
	enum HsqsSuperblockCompressionId id =
			hsqs_data_superblock_compression_id(hsqs->superblock.superblock);
	switch (id) {
	case HSQS_COMPRESSION_NONE:
		fputs("none\n", out);
		break;
	case HSQS_COMPRESSION_GZIP:
		fputs("gzip\n", out);
		handler = compression_info_gzip;
		break;
	case HSQS_COMPRESSION_LZMA:
		fputs("lzma\n", out);
		break;
	case HSQS_COMPRESSION_LZO:
		fputs("lzo\n", out);
		break;
	case HSQS_COMPRESSION_XZ:
		fputs("xz\n", out);
		break;
	case HSQS_COMPRESSION_LZ4:
		fputs("lz4\n", out);
		break;
	case HSQS_COMPRESSION_ZSTD:
		fputs("zstd\n", out);
		break;
	}

	if (handler == NULL) {
		fputs("WARNING: NO COMPRESSION OPTION HANDLER\n", stdout);
	} else if (
			hsqs_data_superblock_flags(hsqs->superblock.superblock) &
			HSQS_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		rv = hsqs_metablock_from_offset(
				&metablock, &hsqs->superblock, sizeof(struct HsqsSuperblock));
		if (rv < 0) {
			goto out;
		}
		metablock_info(metablock, hsqs);
		handler(hsqs);
	} else {
		fputs("NO COMPRESSION OPTION\n", stdout);
	}
out:
	return rv;
}

static int
flag_info(struct Hsqs *hsqs) {
	enum HsqsSuperblockFlags flags =
			hsqs_data_superblock_flags(hsqs->superblock.superblock);
	fputs("=== FLAG INFO ===\n", out);
	KEY("FLAGS");
	printb(flags, out);
#define PRINT_FLAG(x) \
	{ \
		KEY(#x); \
		fputc(flags &HSQS_SUPERBLOCK_##x ? '1' : '0', out); \
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
section_info(struct Hsqs *hsqs) {
	const struct HsqsSuperblock *s = hsqs->superblock.superblock;
	KEY("ID TABLE");
	fprintf(out, "%" PRId64 "\n", hsqs_data_superblock_id_table_start(s));
	KEY("XATTR TABLE");
	fprintf(out, "%" PRId64 "\n", hsqs_data_superblock_xattr_id_table_start(s));
	KEY("INODE TABLE");
	fprintf(out, "%" PRId64 "\n", hsqs_data_superblock_inode_table_start(s));
	KEY("DIR. TABLE");
	fprintf(out, "%" PRId64 "\n",
			hsqs_data_superblock_directory_table_start(s));
	KEY("FRAG TABLE");
	fprintf(out, "%" PRId64 "\n", hsqs_data_superblock_fragment_table_start(s));
	KEY("FRAGMENT ENTRIES");
	fprintf(out, "%u\n", hsqs_data_superblock_fragment_entry_count(s));
	KEY("EXPORT TABLE");
	fprintf(out, "%" PRId64 "\n", hsqs_data_superblock_export_table_start(s));
	return 0;
}

int
main(int argc, char *argv[]) {
	int rv;
	int opt = 0;
	struct Hsqs hsqs = {0};

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

	rv = hsqs_open(&hsqs, argv[optind]);
	if (rv < 0) {
		perror(argv[optind]);
		return EXIT_FAILURE;
	}

	flag_info(&hsqs);
	section_info(&hsqs);
	compression_info(&hsqs);

	root_inode_info(&hsqs);

	hsqs_cleanup(&hsqs);

	return 0;
}
