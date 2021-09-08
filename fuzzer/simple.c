/*
 * plist.c
 * Copyright (C) 2019 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "../src/squash.h"
#include <stdint.h>

int LLVMFuzzerTestOneInput(char *data, size_t size) {
	struct Squash squash = { 0 };
	squash_init(&squash, (uint8_t *)data, size, SQUASH_DTOR_NONE);
	return 0;  // Non-zero return values are reserved for future use.
}
