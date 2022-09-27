/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : common.c
 * @created     : Thursday Sep 08, 2022 11:00:02 CEST
 */

#ifndef BIN_COMMON_H

#define BIN_COMMON_H

#include "../src/sqsh.h"
#include <ctype.h>
#include <string.h>

static int
open_archive(struct Sqsh *sqsh, const char *image_path) {
#ifdef CONFIG_CURL
	int i;
	for (i = 0; isalnum(image_path[i]); i++) {
	}
	if (strncmp(&image_path[i], "://", 3) == 0) {
		return sqsh_open_url(sqsh, image_path);
	}
#endif

	return sqsh_open(sqsh, image_path);
}

#endif /* end of include guard COMMON_H */
