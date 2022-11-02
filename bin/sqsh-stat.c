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
 * @file         sqsh-cat.c
 */

#include "../src/context/inode_context.h"
#include "../src/error.h"
#include "../src/iterator/xattr_iterator.h"
#include "../src/sqsh.h"
#include "common.h"

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char *
compression_id_name(int id) {
	switch (id) {
	case SQSH_COMPRESSION_NONE:
		return "none";
	case SQSH_COMPRESSION_GZIP:
		return "gzip";
	case SQSH_COMPRESSION_XZ:
		return "xz";
	case SQSH_COMPRESSION_LZMA:
		return "lzma";
	case SQSH_COMPRESSION_LZO:
		return "lzo";
	case SQSH_COMPRESSION_LZ4:
		return "lz4";
	case SQSH_COMPRESSION_ZSTD:
		return "zstd";
	default:
		return "unknown";
	}
}
static int
usage(char *arg0) {
	printf("usage: %s FILESYSTEM [PATH ...]\n", arg0);
	printf("       %s -v\n", arg0);
	return EXIT_FAILURE;
}

static int
stat_gzip_options(
		const struct SqshCompressionOptionsContext *compression_options) {
	printf("       compression level: %i\n",
		   sqsh_compression_options_gzip_compression_level(
				   compression_options));
	printf("             window size: %i\n",
		   sqsh_compression_options_gzip_window_size(compression_options));
	const enum SqshGzipStrategies strategies =
			sqsh_compression_options_gzip_strategies(compression_options);
	printf("              strategies: %04x\n", strategies);
	if (strategies & SQSH_GZIP_STRATEGY_DEFAULT) {
		puts("                          default");
	}
	if (strategies & SQSH_GZIP_STRATEGY_FILTERED) {
		puts("                          filtered");
	}
	if (strategies & SQSH_GZIP_STRATEGY_HUFFMAN_ONLY) {
		puts("                          huffman only");
	}
	if (strategies & SQSH_GZIP_STRATEGY_RLE) {
		puts("                          rle");
	}
	if (strategies & SQSH_GZIP_STRATEGY_FIXED) {
		puts("                          fixed");
	}

	return 0;
}

static int
stat_xz_options(
		const struct SqshCompressionOptionsContext *compression_options) {
	printf("         dictionary size: %i\n",
		   sqsh_compression_options_xz_dictionary_size(compression_options));
	const enum SqshXzFilters filters =
			sqsh_compression_options_xz_filters(compression_options);
	printf("                 filters: %04x\n", filters);
	if (filters & SQSH_XZ_FILTER_X86) {
		puts("                          x86");
	}
	if (filters & SQSH_XZ_FILTER_POWERPC) {
		puts("                          powerpc");
	}
	if (filters & SQSH_XZ_FILTER_IA64) {
		puts("                          ia64");
	}
	if (filters & SQSH_XZ_FILTER_ARM) {
		puts("                          arm");
	}
	if (filters & SQSH_XZ_FILTER_ARMTHUMB) {
		puts("                          armthumb");
	}

	return 0;
}

static int
stat_lz4_options(
		const struct SqshCompressionOptionsContext *compression_options) {
	printf("                 version: %i\n",
		   sqsh_compression_options_lz4_version(compression_options));
	const enum SqshLz4Flags flags =
			sqsh_compression_options_lz4_flags(compression_options);
	printf("                   flags: %x\n", flags);
	if (flags & SQSH_LZ4_HIGH_COMPRESSION) {
		puts("                         high compression");
	}

	return 0;
}

static int
stat_zstd_options(
		const struct SqshCompressionOptionsContext *compression_options) {
	printf("       compression level: %i\n",
		   sqsh_compression_options_zstd_compression_level(
				   compression_options));

	return 0;
}

static int
stat_lzo_options(
		const struct SqshCompressionOptionsContext *compression_options) {
	const enum SqshLzoAlgorithm algorithm =
			sqsh_compression_options_lzo_algorithm(compression_options);
	const char *algorithm_str;
	switch (algorithm) {
	case SQSH_LZO_ALGORITHM_LZO1X_1:
		algorithm_str = "lzo1x_1";
		break;
	case SQSH_LZO_ALGORITHM_LZO1X_1_11:
		algorithm_str = "lzo1x_1_11";
		break;
	case SQSH_LZO_ALGORITHM_LZO1X_1_12:
		algorithm_str = "lzo1x_1_12";
		break;
	case SQSH_LZO_ALGORITHM_LZO1X_1_15:
		algorithm_str = "lzo1x_1_15";
		break;
	case SQSH_LZO_ALGORITHM_LZO1X_999:
		algorithm_str = "lzo1x_999";
		break;
	default:
		algorithm_str = "unknown";
		break;
	}
	printf("               algorithm: %s (%x)\n", algorithm_str, algorithm);
	printf("       compression level: %i\n",
		   sqsh_compression_options_lzo_compression_level(compression_options));

	return 0;
}

static int
stat_image(struct Sqsh *sqsh) {
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);
	struct SqshCompressionOptionsContext *compression_options;
	int compression_id = sqsh_superblock_compression_id(superblock);
	int rv = 0;
	printf("             compression: %s (%i)\n",
		   compression_id_name(compression_id), compression_id);
	printf("             inode count: %i\n",
		   sqsh_superblock_inode_count(superblock));
	printf("           uid/gid count: %i\n",
		   sqsh_superblock_id_count(superblock));
	printf("           has fragments: %s\n",
		   sqsh_superblock_has_fragments(superblock) ? "yes" : "no");
	printf("        has export table: %s\n",
		   sqsh_superblock_has_fragments(superblock) ? "yes" : "no");
	printf(" has compression options: %s\n",
		   sqsh_superblock_has_fragments(superblock) ? "yes" : "no");
	printf("              block size: %i\n",
		   sqsh_superblock_block_size(superblock));
	printf("          fragment count: %i\n",
		   sqsh_superblock_fragment_entry_count(superblock));
	printf("            archive size: %" PRIu64 "\n",
		   sqsh_superblock_bytes_used(superblock));
	time_t mtime = sqsh_superblock_modification_time(superblock);
	printf("       modification time: %s\n", ctime(&mtime));

	rv = sqsh_compression_options(sqsh, &compression_options);
	if (rv == -SQSH_ERROR_NO_COMPRESSION_OPTIONS) {
		rv = 0;
	} else if (rv == 0) {
		puts("=== compression options ===");
		switch (compression_id) {
		case SQSH_COMPRESSION_GZIP:
			stat_gzip_options(compression_options);
			break;
		case SQSH_COMPRESSION_XZ:
			stat_xz_options(compression_options);
			break;
		case SQSH_COMPRESSION_LZ4:
			stat_lz4_options(compression_options);
			break;
		case SQSH_COMPRESSION_ZSTD:
			stat_zstd_options(compression_options);
			break;
		case SQSH_COMPRESSION_LZO:
			stat_lzo_options(compression_options);
			break;
		default:
		case SQSH_COMPRESSION_LZMA:
		case SQSH_COMPRESSION_NONE:
			puts("  WARNING: compression options are present, but "
				 "not supported by the compression algorithm");
		}
	}
	return rv;
}

static char *
inode_type_name(int type) {
	switch (type) {
	case SQSH_INODE_TYPE_FILE:
		return "file";
	case SQSH_INODE_TYPE_DIRECTORY:
		return "directory";
	case SQSH_INODE_TYPE_SYMLINK:
		return "symlink";
	case SQSH_INODE_TYPE_FIFO:
		return "fifo";
	case SQSH_INODE_TYPE_SOCKET:
		return "socket";
	case SQSH_INODE_TYPE_BLOCK:
		return "block device";
	case SQSH_INODE_TYPE_CHAR:
		return "character device";
	default:
		return "unknown";
	}
}

static int
stat_file(struct Sqsh *sqsh, const char *path) {
	int rv = 0;
	struct SqshInodeContext inode = {0};
	bool has_fragment = false;
	rv = sqsh_inode_init_by_path(&inode, sqsh, path);
	if (rv < 0) {
		sqsh_perror(rv, path);
		return rv;
	}

	int inode_type = sqsh_inode_type(&inode);
	printf("         inode type: %s\n", inode_type_name(inode_type));
	printf(" extended structure: %s\n",
		   sqsh_inode_is_extended(&inode) ? "yes" : "no");
	printf("       inode number: %i\n", sqsh_inode_number(&inode));
	printf("        permissions: %04o\n", sqsh_inode_permission(&inode));
	printf("                uid: %i\n", sqsh_inode_uid(&inode));
	printf("                gid: %i\n", sqsh_inode_gid(&inode));
	printf("    hard link count: %i\n", sqsh_inode_hard_link_count(&inode));
	printf("          file size: %" PRIu64 "\n", sqsh_inode_file_size(&inode));
	switch (inode_type) {
	case SQSH_INODE_TYPE_FILE:
		has_fragment = sqsh_inode_file_has_fragment(&inode);
		printf("       has fragment: %s\n", has_fragment ? "yes" : "no");
		if (has_fragment) {
			printf("     fragment index: %i\n",
				   sqsh_inode_file_fragment_block_index(&inode));
			printf("    fragment offset: %i\n",
				   sqsh_inode_file_fragment_block_offset(&inode));
		}
		printf("   number of blocks: %i\n",
			   sqsh_inode_file_block_count(&inode));
		for (uint32_t i = 0; i < sqsh_inode_file_block_count(&inode); i++) {
			bool is_compressed = sqsh_inode_file_block_is_compressed(&inode, i);
			uint32_t size = sqsh_inode_file_block_size(&inode, i);

			printf("                    - %i (compressed: %s)\n", size,
				   is_compressed ? "yes" : "no");
		}
		break;
	case SQSH_INODE_TYPE_SYMLINK:
		printf("     symlink target: %.*s\n", sqsh_inode_symlink_size(&inode),
			   sqsh_inode_symlink(&inode));
		break;
	case SQSH_INODE_TYPE_BLOCK:
	case SQSH_INODE_TYPE_CHAR:
		printf("       device major: %i\n",
			   (sqsh_inode_device_id(&inode) & 0xFFF00) >> 8);
		printf("       device minor: %i\n",
			   sqsh_inode_device_id(&inode) & 0xFF);
		break;
	}

	sqsh_inode_cleanup(&inode);
	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *image_path;
	struct Sqsh *sqsh;

	while ((opt = getopt(argc, argv, "vh")) != -1) {
		switch (opt) {
		case 'v':
			puts("sqsh-stat-" VERSION);
			return 0;
		default:
			return usage(argv[0]);
		}
	}

	if (optind + 1 > argc) {
		return usage(argv[0]);
	}

	image_path = argv[optind];
	optind++;

	sqsh = open_archive(image_path, &rv);
	if (rv < 0) {
		sqsh_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	if (optind == argc) {
		rv = stat_image(sqsh);
	} else if (optind + 1 == argc) {
		rv = stat_file(sqsh, argv[optind]);
	} else {
		rv = usage(argv[0]);
	}

out:
	sqsh_free(sqsh);
	return rv;
}
