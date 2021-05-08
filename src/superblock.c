/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock
 * @created     : Friday Apr 30, 2021 12:34:53 CEST
 */

#include "squash.h"
#include "superblock.h"
#include "utils.h"

struct SquashSuperblockWrap *
squash_superblock_wrap(uint8_t *bytes, size_t size) {
	if (size < SQUASH_SUPERBLOCK_SIZE) {
		return NULL;
	}

	struct SquashSuperblockWrap *superblock =
		(struct SquashSuperblockWrap *)bytes;

	if (superblock->magic != htole32(SQUASH_SUPERBLOCK_MAGIC)) {
		return NULL;
	}

	ENSURE_HOST_ORDER_32(superblock->magic);
	ENSURE_HOST_ORDER_32(superblock->inode_count);
	ENSURE_HOST_ORDER_32(superblock->modification_time);
	ENSURE_HOST_ORDER_32(superblock->block_size);
	ENSURE_HOST_ORDER_32(superblock->fragment_entry_count);
	ENSURE_HOST_ORDER_16(superblock->compression_id);
	ENSURE_HOST_ORDER_16(superblock->block_log);
	ENSURE_HOST_ORDER_16(superblock->flags);
	ENSURE_HOST_ORDER_16(superblock->id_count);
	ENSURE_HOST_ORDER_16(superblock->version_major);
	ENSURE_HOST_ORDER_16(superblock->version_minor);
	ENSURE_HOST_ORDER_64(superblock->root_inode_ref);
	ENSURE_HOST_ORDER_64(superblock->bytes_used);
	ENSURE_HOST_ORDER_64(superblock->id_table_start);
	ENSURE_HOST_ORDER_64(superblock->xattr_id_table_start);
	ENSURE_HOST_ORDER_64(superblock->inode_table_start);
	ENSURE_HOST_ORDER_64(superblock->directory_table_start);
	ENSURE_HOST_ORDER_64(superblock->fragment_table_start);
	ENSURE_HOST_ORDER_64(superblock->export_table_start);

	return superblock;
}

uint8_t *
squash_superblock_unwrap(struct SquashSuperblockWrap *superblock) {
	ENSURE_FORMAT_ORDER_32(superblock->magic);
	ENSURE_FORMAT_ORDER_32(superblock->inode_count);
	ENSURE_FORMAT_ORDER_32(superblock->modification_time);
	ENSURE_FORMAT_ORDER_32(superblock->block_size);
	ENSURE_FORMAT_ORDER_32(superblock->fragment_entry_count);
	ENSURE_FORMAT_ORDER_16(superblock->compression_id);
	ENSURE_FORMAT_ORDER_16(superblock->block_log);
	ENSURE_FORMAT_ORDER_16(superblock->flags);
	ENSURE_FORMAT_ORDER_16(superblock->id_count);
	ENSURE_FORMAT_ORDER_16(superblock->version_major);
	ENSURE_FORMAT_ORDER_16(superblock->version_minor);
	ENSURE_FORMAT_ORDER_64(superblock->root_inode_ref);
	ENSURE_FORMAT_ORDER_64(superblock->bytes_used);
	ENSURE_FORMAT_ORDER_64(superblock->id_table_start);
	ENSURE_FORMAT_ORDER_64(superblock->xattr_id_table_start);
	ENSURE_FORMAT_ORDER_64(superblock->inode_table_start);
	ENSURE_FORMAT_ORDER_64(superblock->directory_table_start);
	ENSURE_FORMAT_ORDER_64(superblock->fragment_table_start);
	ENSURE_FORMAT_ORDER_64(superblock->export_table_start);

	return (uint8_t *)superblock;
}
