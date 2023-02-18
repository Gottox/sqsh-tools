/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

#include "common.h"

#include "../include/sqsh_data.h"
#include "../include/sqsh_data_private.h"
#include "../include/sqsh_private.h"
#include "sqsh_context.h"

#include <assert.h>

uint8_t *
mk_stub(struct Sqsh *sqsh, uint8_t *payload, size_t payload_size) {
	struct SqshDataSuperblock superblock = {
			.magic = SQSH_SUPERBLOCK_MAGIC,
			.inode_count = 0,
			.modification_time = 0,
			.block_size = 4096,
			.fragment_entry_count = 0,
			.compression_id = SQSH_COMPRESSION_GZIP,
			.block_log = 12,
			.flags = 0,
			.id_count = 0,
			.version_major = 4,
			.version_minor = 0,
			.root_inode_ref = 0,
			.bytes_used = SQSH_SIZEOF_SUPERBLOCK + payload_size,
			.id_table_start = 0,
			.xattr_id_table_start = 0,
			.inode_table_start = 0,
			.directory_table_start = 0,
			.fragment_table_start = 0,
			.export_table_start = 0,
	};

	uint8_t *data = (uint8_t *)malloc(SQSH_SIZEOF_SUPERBLOCK + payload_size);
	memcpy(data, &superblock, SQSH_SIZEOF_SUPERBLOCK);
	memcpy(&data[SQSH_SIZEOF_SUPERBLOCK], payload, payload_size);

	const struct SqshConfig config = {
			.source_size = SQSH_SIZEOF_SUPERBLOCK + payload_size,
			.source_type = SQSH_SOURCE_TYPE_MEMORY};
	assert(0 == sqsh__init(sqsh, data, &config));
	return data;
}
