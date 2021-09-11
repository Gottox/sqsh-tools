/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : superblock_internal
 * @created     : Friday Sep 10, 2021 09:25:32 CEST
 */

#include "superblock.h"

#ifndef SUPERBLOCK_INTERNAL_H

#define SUPERBLOCK_INTERNAL_H

struct SquashSuperblock {
	uint32_t magic;
	uint32_t inode_count;
	uint32_t modification_time;
	uint32_t block_size;
	uint32_t fragment_entry_count;
	uint16_t compression_id;
	uint16_t block_log;
	uint16_t flags;
	uint16_t id_count;
	uint16_t version_major;
	uint16_t version_minor;
	uint64_t root_inode_ref;
	uint64_t bytes_used;
	uint64_t id_table_start;
	uint64_t xattr_id_table_start;
	uint64_t inode_table_start;
	uint64_t directory_table_start;
	uint64_t fragment_table_start;
	uint64_t export_table_start;
};

#endif /* end of include guard SUPERBLOCK_INTERNAL_H */
