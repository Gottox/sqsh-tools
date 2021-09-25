/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:44:12 CEST
 */

#include "fragment_context.h"
#include "../data/fragment.h"
#include "../data/superblock.h"
#include <stdint.h>

int
squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode) {
	int rv = 0;
	if (squash_data_superblock_flags(superblock) &
			SQUASH_SUPERBLOCK_NO_FRAGMENTS) {
		goto out;
	}

	rv = squash_metablock_init(&fragment->table, superblock,
			squash_data_superblock_fragment_table_start(superblock));
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

uint64_t
squash_fragment_start(struct SquashFragmentContext *fragment) {
	return 0;
}

uint32_t
squash_fragment_size(struct SquashFragmentContext *fragment) {
	return squash_data_datablock_size(
			squash_data_fragment_size_info(fragment->fragment));
}

const uint8_t *
squash_fragment_data(struct SquashFragmentContext *fragment) {
	return 0;
}

int
squash_fragment_clean(struct SquashFragmentContext *fragment) {
	squash_metablock_cleanup(&fragment->table);

	return 0;
}
