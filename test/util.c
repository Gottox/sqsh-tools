/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

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
	uint8_t superblock[SQSH_SIZEOF_SUPERBLOCK] = {
			UINT32_BYTES(SQSH_SUPERBLOCK_MAGIC), // magic
			UINT32_BYTES(0), // inode_count
			UINT32_BYTES(0), // modification_time
			UINT32_BYTES(4096), // block_size
			UINT32_BYTES(0), // fragment_entry_count
			UINT16_BYTES(SQSH_COMPRESSION_GZIP), // compression_id
			UINT16_BYTES(12), // block_log
			UINT16_BYTES(0), // flags
			UINT16_BYTES(0), // id_count
			UINT16_BYTES(4), // version_major
			UINT16_BYTES(0), // version_minor
			UINT64_BYTES((uint64_t)0), // root_inode_ref
			UINT64_BYTES(payload_size), // bytes_used
			UINT64_BYTES((uint64_t)0), // id_table_start
			UINT64_BYTES((uint64_t)0), // xattr_id_table_start
			UINT64_BYTES((uint64_t)0), // inode_table_start
			UINT64_BYTES((uint64_t)0), // directory_table_start
			UINT64_BYTES((uint64_t)0), // fragment_table_start
			UINT64_BYTES((uint64_t)0), // export_table_start
	};

	memcpy(payload, &superblock, SQSH_SIZEOF_SUPERBLOCK);

	const struct SqshConfig config = DEFAULT_CONFIG(payload_size);
	rv = sqsh__archive_init(sqsh, payload, &config);
	assert(0 == rv);
	(void)rv;
	return payload;
}
