/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:44:12 CEST
 */

#include "fragment_context.h"
#include "../data/fragment.h"
#include "../extract.h"
#include "metablock_context.h"
#include <stdint.h>

int
squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblock *superblock) {
	int rv = 0;
	if (squash_data_superblock_flags(superblock) &
			SQUASH_SUPERBLOCK_NO_FRAGMENTS) {
		fragment->fragment_count = 0;
		fragment->table = 0;
		return rv;
	}

	fragment->table = squash_metablock_from_offset(superblock,
			squash_data_superblock_fragment_table_start(superblock));
	return rv;
}

const uint32_t *
squash_fragment_get_start(
		struct SquashFragmentContext *fragment, uint32_t index) {
	int rv = 0;
	size_t size = (index + 1) * SQUASH_SIZEOF_FRAGMENT;
	rv = squash_extract_more(&fragment->extract, size);
	if (rv < 0) {
		return NULL;
	}
	return NULL;
}

uint32_t
squash_fragment_count(struct SquashFragmentContext *fragment) {
	return fragment->fragment_count;
}

int
squash_fragment_clean(struct SquashFragmentContext *fragment) {
	squash_extract_cleanup(&fragment->extract);

	return 0;
}
