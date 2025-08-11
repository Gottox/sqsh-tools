/******************************************************************************
 *                                                                            *
 * Copyright (c) 2024, Enno Boland <g@s01.de>                                 *
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
 * @file         mk.c
 */

#include <sqshtools_common.h>

#include <mksqsh_archive_private.h>
#include <mksqsh_file.h>

#include <stdio.h>
#include <string.h>

static int
usage(char *arg0) {
        printf("usage: %s FILE... ARCHIVE\n", arg0);
        printf("       %s -v\n", arg0);
        return EXIT_FAILURE;
}

static const char opts[] = "vh";
static const struct option long_opts[] = {
                {"version", no_argument, NULL, 'v'},
                {"help", no_argument, NULL, 'h'},
                {0},
};

int
main(int argc, char *argv[]) {
        int opt;
        while ((opt = getopt_long(argc, argv, opts, long_opts, NULL)) != -1) {
                switch (opt) {
                case 'v':
                        puts("sqsh-mk-" VERSION);
                        return 0;
                default:
                        return usage(argv[0]);
                }
        }

        if (argc - optind < 2) {
                return usage(argv[0]);
        }

        int rv = 0;
        struct MksqshArchive archive = {0};
        rv = mksqsh_archive_init(&archive);
        if (rv < 0) {
                sqsh_perror(rv, "mksqsh_archive_init");
                return EXIT_FAILURE;
        }

        struct MksqshFile *root = mksqsh_archive_root(&archive);
        if (root == NULL) {
                rv = EXIT_FAILURE;
                goto out;
        }

        for (int i = optind; i < argc - 1; i++) {
                const char *path = argv[i];
                struct MksqshFile *file =
                                mksqsh_file_new(&archive, MKSQSH_FILE_TYPE_REG, &rv);
                if (rv < 0 || file == NULL) {
                        sqsh_perror(rv, path);
                        rv = EXIT_FAILURE;
                        goto out;
                }
                mksqsh_file_content_from_path(file, path, &rv);
                if (rv < 0) {
                        sqsh_perror(rv, path);
                        mksqsh_file_release(file);
                        rv = EXIT_FAILURE;
                        goto out;
                }
                const char *name = strrchr(path, '/');
                name = (name == NULL) ? path : name + 1;
                rv = mksqsh_file_add(root, name, file);
                mksqsh_file_release(file);
                if (rv < 0) {
                        sqsh_perror(rv, path);
                        rv = EXIT_FAILURE;
                        goto out;
                }
        }

        const char *archive_path = argv[argc - 1];
        mksqsh_file_release(root);
        rv = mksqsh_archive_write(&archive, archive_path);
        if (rv < 0) {
                sqsh_perror(rv, archive_path);
                rv = EXIT_FAILURE;
                goto out;
        }
        rv = 0;

out:
        mksqsh_archive_cleanup(&archive);
        return rv;
}

