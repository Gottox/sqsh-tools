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
 * @created     : Monday Sep 20, 2021 09:48:58 CEST
 */

#include "../src/context/content_context.h"
#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/hsqs.h"
#include "../src/resolve_path.h"

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
	struct HsqsInodeContext inode = {0};
	struct HsqsFileContext file = {0};
	struct Hsqs hsqs = {0};

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

	rv = hsqs_open(&hsqs, outer_path);
	if (rv < 0) {
		hsqs_perror(rv, outer_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = hsqs_resolve_path(&inode, &hsqs.superblock, inner_path);
	if (rv < 0) {
		hsqs_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = hsqs_file_init(&file, &inode);
	if (rv < 0) {
		hsqs_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	rv = hsqs_file_read(&file, hsqs_inode_file_size(&inode));
	if (rv < 0) {
		hsqs_perror(rv, inner_path);
		rv = EXIT_FAILURE;
		goto out;
	}

	fwrite(hsqs_file_data(&file), sizeof(uint8_t), hsqs_file_size(&file),
		   stdout);

out:
	hsqs_file_cleanup(&file);
	hsqs_inode_cleanup(&inode);

	hsqs_cleanup(&hsqs);

	return rv;
}
