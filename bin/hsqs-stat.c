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
 * @file         hsqs-cat.c
 */

#include "../src/context/content_context.h"
#include "../src/context/inode_context.h"
#include "../src/error.h"
#include "../src/hsqs.h"
#include "../src/iterator/xattr_iterator.h"

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
	case HSQS_COMPRESSION_NONE:
		return "none";
	case HSQS_COMPRESSION_GZIP:
		return "gzip";
	case HSQS_COMPRESSION_XZ:
		return "xz";
	case HSQS_COMPRESSION_LZMA:
		return "lzma";
	case HSQS_COMPRESSION_LZO:
		return "lzo";
	case HSQS_COMPRESSION_LZ4:
		return "lz4";
	case HSQS_COMPRESSION_ZSTD:
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
stat_image(struct Hsqs *hsqs) {
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	int compression_id = hsqs_superblock_compression_id(superblock);
	printf("compression:              %s (%i)\n",
		   compression_id_name(compression_id), compression_id);
	printf("inode count:              %i\n",
		   hsqs_superblock_inode_count(superblock));
	printf("uid/gid count:            %i\n",
		   hsqs_superblock_id_count(superblock));
	printf("has fragments:            %s\n",
		   hsqs_superblock_has_fragments(superblock) ? "yes" : "no");
	printf("has export table:         %s\n",
		   hsqs_superblock_has_fragments(superblock) ? "yes" : "no");
	printf("has compression options:  %s\n",
		   hsqs_superblock_has_fragments(superblock) ? "yes" : "no");
	printf("block size:               %i\n",
		   hsqs_superblock_block_size(superblock));
	printf("fragment count:           %i\n",
		   hsqs_superblock_fragment_entry_count(superblock));
	printf("archive size:             %" PRIu64 "\n",
		   hsqs_superblock_bytes_used(superblock));
	time_t mtime = hsqs_superblock_modification_time(superblock);
	printf("modification time:        %s\n", ctime(&mtime));
	return 0;
}

static char *
inode_type_name(int type) {
	switch (type) {
	case HSQS_INODE_TYPE_FILE:
		return "file";
	case HSQS_INODE_TYPE_DIRECTORY:
		return "directory";
	case HSQS_INODE_TYPE_SYMLINK:
		return "symlink";
	case HSQS_INODE_TYPE_FIFO:
		return "fifo";
	case HSQS_INODE_TYPE_SOCKET:
		return "socket";
	case HSQS_INODE_TYPE_BLOCK:
		return "block device";
	case HSQS_INODE_TYPE_CHAR:
		return "character device";
	default:
		return "unknown";
	}
}

static int
stat_file(struct Hsqs *hsqs, const char *path) {
	int rv = 0;
	struct HsqsInodeContext inode = {0};
	struct HsqsXattrIterator iter = {0};
	(void)hsqs;
	(void)path;
	rv = hsqs_inode_load_by_path(&inode, hsqs, path);
	if (rv < 0) {
		hsqs_perror(rv, path);
		return rv;
	}

	int inode_type = hsqs_inode_type(&inode);
	printf("         inode type: %s\n", inode_type_name(inode_type));
	printf(" extended structure: %s\n",
		   hsqs_inode_is_extended(&inode) ? "yes" : "no");
	printf("       inode number: %i\n", hsqs_inode_number(&inode));
	printf("        permissions: %04o\n", hsqs_inode_permission(&inode));
	printf("                uid: %i\n", hsqs_inode_uid(&inode));
	printf("                gid: %i\n", hsqs_inode_gid(&inode));
	printf("    hard link count: %i\n", hsqs_inode_hard_link_count(&inode));
	printf("          file size: %" PRIu64 "\n", hsqs_inode_file_size(&inode));
	switch (inode_type) {
	case HSQS_INODE_TYPE_FILE:
		printf("       has fragment: %s\n",
			   hsqs_inode_file_has_fragment(&inode) ? "yes" : "no");
		printf("   number of blocks: %i\n",
			   hsqs_inode_file_block_count(&inode));
		for (uint32_t i = 0; i < hsqs_inode_file_block_count(&inode); i++) {
			bool is_compressed = hsqs_inode_file_block_is_compressed(&inode, i);
			uint32_t size = hsqs_inode_file_block_size(&inode, i);

			printf("                    - %i (compressed: %s)\n", size,
				   is_compressed ? "yes" : "no");
		}
		break;
	case HSQS_INODE_TYPE_SYMLINK:
		printf("     symlink target: %.*s\n", hsqs_inode_symlink_size(&inode),
			   hsqs_inode_symlink(&inode));
		break;
	case HSQS_INODE_TYPE_BLOCK:
	case HSQS_INODE_TYPE_CHAR:
		printf("       device major: %i\n",
			   (hsqs_inode_device_id(&inode) & 0xFFF00) >> 8);
		printf("       device minor: %i\n",
			   hsqs_inode_device_id(&inode) & 0xFF);
		break;
	}

	rv = hsqs_inode_xattr_iterator(&inode, &iter);
	if (rv < 0) {
		hsqs_perror(rv, "hsqs_inode_xattr_iterator");
		return rv;
	}
	if ((rv = hsqs_xattr_iterator_next(&iter)) > 0) {
		printf("xattrs:\n");
		do {
			const char *name = hsqs_xattr_iterator_name(&iter);
			const char *value = hsqs_xattr_iterator_value(&iter);
			printf("    %s = %s\n", name, value);
		} while ((rv = hsqs_xattr_iterator_next(&iter)) > 0);
	}

	hsqs_inode_cleanup(&inode);
	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	int opt = 0;
	const char *image_path;
	struct Hsqs hsqs = {0};

	while ((opt = getopt(argc, argv, "vh")) != -1) {
		switch (opt) {
		case 'v':
			puts("hsqs-stat-" VERSION);
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

	rv = hsqs_open(&hsqs, image_path);
	if (rv < 0) {
		hsqs_perror(rv, image_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	if (optind == argc) {
		rv = stat_image(&hsqs);
	} else if (optind + 1 == argc) {
		rv = stat_file(&hsqs, argv[optind]);
	} else {
		rv = usage(argv[0]);
	}

out:
	hsqs_cleanup(&hsqs);
	return rv;
}
