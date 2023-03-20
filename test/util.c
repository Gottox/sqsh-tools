/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

#define _DEFAULT_SOURCE

#include "common.h"

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_data.h"
#include "../include/sqsh_data_private.h"
#include <sqsh_mapper.h>

#include <assert.h>
#include <endian.h>

const uint8_t *
mk_stub(struct SqshArchive *sqsh, uint8_t *payload, size_t payload_size) {
	int rv;
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
			.bytes_used = htole64(payload_size),
			.id_table_start = 0,
			.xattr_id_table_start = 0,
			.inode_table_start = 0,
			.directory_table_start = 0,
			.fragment_table_start = 0,
			.export_table_start = 0,
	};

	memcpy(payload, &superblock, SQSH_SIZEOF_SUPERBLOCK);

	const struct SqshConfig config = DEFAULT_CONFIG(payload_size);
	rv = sqsh__archive_init(sqsh, payload, &config);
	assert(0 == rv);
	(void)rv;
	return payload;
}
