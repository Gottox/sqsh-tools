/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : common.h
 */

#ifndef BIN_COMMON_H

#define BIN_COMMON_H

#include "../src/sqsh.h"
#include <ctype.h>
#include <string.h>

static int
open_archive(struct Sqsh *sqsh, const char *image_path) {
	struct SqshConfig config = {
			.source_type = SQSH_SOURCE_TYPE_PATH,
	};
#ifdef CONFIG_CURL
	int i;
	for (i = 0; isalnum(image_path[i]); i++) {
	}
	if (strncmp(&image_path[i], "://", 3) == 0) {
		config.source_type = SQSH_SOURCE_TYPE_CURL;
	}
#endif

	return sqsh_init(sqsh, image_path, &config);
}

#endif /* end of include guard COMMON_H */
