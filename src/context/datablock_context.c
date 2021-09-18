/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : datablock_context
 * @created     : Tuesday Sep 14, 2021 22:56:07 CEST
 */

#include "datablock_context.h"
#include "../data/inode.h"
#include "../data/metablock.h"
#include "../data/superblock.h"
#include "../error.h"
#include <stdint.h>

int
squash_datablock_init(struct SquashDatablockContext *datablock,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode) {
	int rv = 0;
	if (rv < 0)
		goto out;
	datablock->inode = inode;
	datablock->datablock_start =
			(const uint8_t *)superblock + SQUASH_SIZEOF_SUPERBLOCK;
	if (squash_data_superblock_flags(superblock) &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		const struct SquashMetablock *metablock = squash_metablock_from_offset(
				superblock, SQUASH_SIZEOF_SUPERBLOCK);
		if (metablock == NULL) {
			return -SQUASH_ERROR_TODO;
		}
		datablock->datablock_start +=
				SQUASH_SIZEOF_METABLOCK + squash_data_metablock_size(metablock);
	}

	datablock->blocks_start = squash_inode_file_blocks_start(inode);
	datablock->fragment_start = (const uint8_t *)superblock +
			squash_data_superblock_fragment_table_start(superblock);

	rv = squash_compression_init(&datablock->compression, superblock);
out:
	return rv;
}

/*static const uint8_t *
datablock_get_block(struct SquashDatablockContext *datablock, uint64_t index) {
	struct SquashInodeContext *inode = datablock->inode;
	off_t offset = 0;

	for (int i = 0; i < index; i++) {
		offset += squash_inode_file_block_size(inode, i);
	}

	return &datablock->datablock_start[offset];
}

static const uint8_t *
datablock_get_fragment(
		struct SquashDatablockContext *datablock, uint32_t *offset) {
	if (squash_inode_file_fragment_block_index(datablock->inode) ==
			SQUASH_INODE_NO_FRAGMENT)
		return NULL;

	return NULL; // XXX
}*/

int
squash_datablock_decompress(
		struct SquashDatablockContext *datablock, uint64_t index) {
	return 0;
}

int
squash_datablock_clean(struct SquashDatablockContext *datablock) {
	return squash_compression_cleanup(&datablock->compression);
}
