/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : common
 * @created     : Wednesday Jun 07, 2023 15:41:30 CEST
 */

#include <sqshtools_common.h>

struct SqshArchive *
open_archive(const char *image_path, uint64_t offset, int *err) {
	struct SqshConfig config = {
			.source_mapper = NULL,
			.mapper_block_size = 1024 * 256,
			.archive_offset = offset,
	};
	if (sqsh_mapper_impl_curl != NULL) {
		int i;
		for (i = 0; isalnum(image_path[i]); i++) {
		}
		if (strncmp(&image_path[i], "://", 3) == 0) {
			config.source_mapper = sqsh_mapper_impl_curl;
		}
	}

	return sqsh_archive_open(image_path, &config, err);
}
