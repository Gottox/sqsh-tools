/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

#include "common.h"

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_data_private.h"
#include "../include/sqsh_mapper.h"

#include <assert.h>

const uint8_t *
mk_stub(struct SqshArchive *sqsh, uint8_t *payload, size_t payload_size) {
	int rv;
	uint8_t superblock[SQSH_SIZEOF_SUPERBLOCK] = {
			/* magic */
			UINT32_BYTES(SQSH_SUPERBLOCK_MAGIC),
			/* inode_count */
			UINT32_BYTES(100),
			/* modification_time */
			UINT32_BYTES(0),
			/* block_size */
			UINT32_BYTES(32768),
			/* fragment_entry_count */
			UINT32_BYTES(0),
			/* compression_id */
			UINT16_BYTES(SQSH_COMPRESSION_GZIP),
			/* block_log */
			UINT16_BYTES(15),
			/* flags */
			UINT16_BYTES(0),
			/* id_count */
			UINT16_BYTES(0),
			/* version_major */
			UINT16_BYTES(4),
			/* version_minor */
			UINT16_BYTES(0),
			/* root_inode_ref */
			UINT64_BYTES((uint64_t)0),
			/* bytes_used */
			UINT64_BYTES((uint64_t)payload_size),
			/* id_table_start */
			UINT64_BYTES((uint64_t)ID_TABLE_OFFSET),
			UINT64_BYTES(
					/* xattr_id_table_start */
					(uint64_t)XATTR_TABLE_OFFSET),
			/* inode_table_start */
			UINT64_BYTES((uint64_t)INODE_TABLE_OFFSET),
			/* Setting the directory table start to the end
			 * of the archive so the inode tests don't fail */
			UINT64_BYTES((uint64_t)
						 /* directory_table_start */
						 DIRECTORY_TABLE_OFFSET),
			/* fragment_table_start */
			UINT64_BYTES((uint64_t)FRAGMENT_TABLE_OFFSET),
			/* export_table_start */
			UINT64_BYTES((uint64_t)EXPORT_TABLE_OFFSET),
	};

	memcpy(payload, &superblock, SQSH_SIZEOF_SUPERBLOCK);

	const struct SqshConfig config = DEFAULT_CONFIG(payload_size);
	rv = sqsh__archive_init(sqsh, payload, &config);
	assert(0 == rv);
	(void)rv;
	return payload;
}
