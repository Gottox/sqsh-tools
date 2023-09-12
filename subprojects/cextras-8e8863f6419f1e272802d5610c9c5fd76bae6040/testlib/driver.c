/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : driver
 * @created     : Tuesday Jul 25, 2023 13:20:23 CEST
 */

#define _XOPEN_SOURCE

#include <fnmatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "include/testlib.h"

extern const struct TestlibTest testlib_tests[];

static char program_name[4096] = {0};
static const char *color_bad = "";
static const char *color_status = "";
static const char *color_good = "";
static const char *color_reset = "";
static char **patterns = NULL;
static int pattern_count = 0;
static bool verbose = false;

static void
color_on(char *opt) {
	bool enable;

	if (strcmp("always", opt) == 0) {
		enable = true;
	} else if (strcmp("never", opt) == 0) {
		enable = false;
	} else if (strcmp("auto", opt) == 0) {
		enable = isatty(STDERR_FILENO);
	} else {
		fprintf(stderr, "%s: invalid argument '%s' for -c\n", program_name,
				opt);
		exit(EXIT_FAILURE);
	}

	if (enable) {
		color_bad = "\x1b[31;1m";
		color_status = "\x1b[33m";
		color_good = "\x1b[32;1m";
		color_reset = "\x1b[0m";
	} else {
		color_bad = "";
		color_status = "";
		color_good = "";
		color_reset = "";
	}
}

static int
run_test_direct(const struct TestlibTest *test) {
	bool found = true;
	for (int i = 0; i < pattern_count; i++) {
		found = false;
		if (fnmatch(patterns[i], test->name, 0) == 0) {
			found = true;
			break;
		}
	}

	if (found == false) {
		if (verbose) {
			fprintf(stderr, "%s -n '%s'\n IGNORED\n", program_name, test->name);
		}
	} else if (test->enabled == false) {
		fprintf(stderr, "%s -n '%s'\n DISABLED\n", program_name, test->name);
	} else {
		clock_t time = clock();
		fprintf(stderr, "%s%s -n '%s'%s\n", color_reset, program_name,
				test->name, color_status);
		test->func();
		fprintf(stderr, "%s finished in %.3lfms\n", color_reset,
				(double)(clock() - time) * 1000.0 / (double)CLOCKS_PER_SEC);
	}
	return 0;
}

static int
run_test_forked(const struct TestlibTest *test) {
	int pid = fork();
	if (pid > 0) {
		int exitcode = -1;
		int status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status)) {
			exitcode = WEXITSTATUS(status);
		}
		return exitcode;
	} else if (pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	run_test_direct(test);

	exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[]) {
	bool non_fork = false;
	strncpy(program_name, argv[0], sizeof(program_name) - 1);
	int opt;
	int rv = 0;

	color_on("auto");
	while ((opt = getopt(argc, argv, "nlcv:")) != -1) {
		switch (opt) {
		case 'n':
			non_fork = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'c':
			color_on(optarg);
			break;
		case 'l':
			for (int i = 0; testlib_tests[i].name != NULL; i++) {
				printf("%s\n", testlib_tests[i].name);
			}
			exit(EXIT_SUCCESS);
		default:
			fprintf(stderr,
					"Usage: %s [-c always|never|auto] [-e] [-l] [test...]\n",
					program_name);
			exit(EXIT_FAILURE);
		}
	}

	patterns = &argv[optind];
	pattern_count = argc - optind;

	int (*run_test)(const struct TestlibTest *);
	if (non_fork) {
		run_test = run_test_direct;
	} else {
		run_test = run_test_forked;
	}
	for (int i = 0; testlib_tests[i].name != NULL; i++) {
		const struct TestlibTest *test = &testlib_tests[i];

		rv |= run_test(test);
		if (rv != 0 && non_fork) {
			break;
		}
	}

	return rv;
}
