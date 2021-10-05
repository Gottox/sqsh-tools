/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:44:12 CEST
 */

#include "fragment_context.h"
#include "../data/fragment.h"
#include "../data/superblock.h"
#include "../error.h"
#include "inode_context.h"
#include <stdint.h>

static int
uncompress_fragment(struct SquashFragmentContext *fragment) {
	const struct SquashDatablockSize *size_info =
			squash_data_fragment_size_info(fragment->fragment);
	bool is_compressed = squash_data_datablock_is_compressed(size_info);
	uint32_t size = squash_data_datablock_size(size_info);
	uint64_t start = squash_data_fragment_start(fragment->fragment);
	const uint8_t *tmp = (const uint8_t *)fragment->superblock;

	return squash_buffer_append(
			&fragment->buffer, &tmp[start], size, is_compressed);
}

int
squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode) {
	int rv = 0;
	if (squash_data_superblock_flags(superblock) &
			SQUASH_SUPERBLOCK_NO_FRAGMENTS) {
		rv = -SQUASH_ERROR_NO_FRAGMENT;
		goto out;
	}
	fragment->inode = inode;
	fragment->superblock = superblock;

	uint32_t fragment_table_count =
			squash_data_superblock_fragment_entry_count(superblock);
	uint32_t fragment_table_start =
			squash_data_superblock_fragment_table_start(superblock);
	rv = squash_table_init(&fragment->table, superblock, fragment_table_start,
			SQUASH_SIZEOF_FRAGMENT, fragment_table_count);
	if (rv < 0) {
		goto out;
	}
	uint32_t index = squash_inode_file_fragment_block_index(inode);
	rv = squash_table_get(
			&fragment->table, index, (const void **)&fragment->fragment);
	if (rv < 0) {
		goto out;
	}

	uint32_t block_size = squash_data_superblock_block_size(superblock);
	rv = squash_buffer_init(&fragment->buffer, superblock, block_size);
	if (rv < 0) {
		goto out;
	}

	rv = uncompress_fragment(fragment);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		squash_fragment_clean(fragment);
	}
	return rv;
}

uint32_t
squash_fragment_size(struct SquashFragmentContext *fragment) {
	uint64_t file_size = squash_inode_file_size(fragment->inode);
	uint32_t block_size =
			squash_data_superblock_block_size(fragment->superblock);

	return file_size % block_size;
}

const uint8_t *
squash_fragment_data(struct SquashFragmentContext *fragment) {
	const uint8_t *data = squash_buffer_data(&fragment->buffer);
	const uint32_t offset =
			squash_inode_file_fragment_block_offset(fragment->inode);

	return &data[offset];
}

int
squash_fragment_clean(struct SquashFragmentContext *fragment) {
	squash_table_cleanup(&fragment->table);
	squash_buffer_cleanup(&fragment->buffer);

	return 0;
}
