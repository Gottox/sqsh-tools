/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         common.h
 */

#ifndef TEST_H
#define TEST_H

#ifdef NDEBUG
#	undef NDEBUG
#	define _NDEBUG
#endif

#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <assert.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static char T_progname[4096] = {0};
static const char *T_current = NULL;
static const char *T_color_bad = "";
static const char *T_color_status = "";
static const char *T_color_good = "";
static const char *T_color_reset = "";

static int T_test_def(const char *needle);

int
main(int argc, char **argv) {
	if (isatty(STDERR_FILENO)) {
		T_color_bad = "\x1b[31;1m";
		T_color_status = "\x1b[33m";
		T_color_good = "\x1b[32;1m";
		T_color_reset = "\x1b[0m";
	}
	strcpy(T_progname, argv[0]);
	return T_test_def(argc < 2 ? NULL : argv[1]);
}

#ifdef _NDEBUG
#	define ASSERT_ABRT(x) \
		{ x; }
#else
#	define ASSERT_ABRT(x) \
		{ \
			switch (fork()) { \
			case 0: \
				close(STDERR_FILENO); \
				{ x; } \
				exit(0); \
			default: { \
				int s; \
				wait(&s); \
				if (WIFSIGNALED(s) == 0) \
					assert("Abort expected" && 0); \
			} \
			} \
		}
#endif

static void
run_test(void (*func)(void), const char *name) {
	clock_t time = clock();
	T_current = name;
	fprintf(stderr, "%s%s '%s'%s\n", T_color_reset, T_progname, name,
			T_color_status);
	func();
	fprintf(stderr, "%s finished in %lfms\n", T_color_reset,
			(double)(clock() - time) * 1000.0 / (double)CLOCKS_PER_SEC);
}

#define DEFINE static int T_test_def(const char *needle) {
#define TEST(x) \
	if (needle == NULL || strstr(#x, needle)) \
	run_test(x, #x)
#define TEST_OFF(x) \
	(void)x; \
	fprintf(stderr, "%s '%s'\n IGNORED\n", T_progname, #x);

#define DEFINE_END \
	fprintf(stderr, "%s%s is OK!%s\n\n", T_color_good, T_progname, \
			T_color_reset); \
	return 0; \
	}

#endif /* !TEST_H */
