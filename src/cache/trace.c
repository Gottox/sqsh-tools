/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         trace.c
 */

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STACK_POS 4

static void __attribute__((no_instrument_function)) print_func(char *action) {
	void *array[100];
	int size;
	char **strings;

	size = backtrace(array, 100);
	strings = backtrace_symbols(array, size);

	int i;
	for (i = 0; i < size; i++) {
		if (strstr(strings[i], "__cyg_profile_func")) {
			i++;
			break;
		}
	}
	if (i < size) {
		printf("%s %s\n", action, strings[i]);
	}
	free(strings);
}

void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *func, void *caller) {
	(void)func;
	(void)caller;
	print_func("=>");
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *func, void *caller) {
	(void)func;
	(void)caller;
	print_func("<=");
}
