/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

// for fmemopen
#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <sqsh_archive_builder.h>
#include <sqsh_archive_private.h>
#include <sqsh_data_private.h>
#include <sqsh_mapper.h>

const uint8_t *
mk_stub(struct SqshArchive *sqsh, uint8_t *payload, size_t payload_size) {
	int rv;
	const int compression_id =
			payload[20] ? payload[20] : SQSH_COMPRESSION_GZIP;
	struct SqshSuperblockBuilder builder = {0};
	FILE *fsuperblock =
			fmemopen(payload, sizeof(struct SqshDataSuperblock), "w");
	assert(fsuperblock);
	rv = sqsh__superblock_builder_init(&builder);
	assert(rv == 0);
	rv = sqsh__superblock_builder_fragment_count(&builder, 0);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compress_inodes(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compress_data(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compress_fragments(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_force_fragments(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_deduplicate(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compress_xattr(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compression_options(&builder, true);
	assert(rv == 0);
	rv = sqsh__superblock_builder_id_count(&builder, 0);
	assert(rv == 0);
	rv = sqsh__superblock_builder_inode_count(&builder, 100);
	assert(rv == 0);
	rv = sqsh__superblock_builder_root_inode_ref(&builder, 0);
	assert(rv == 0);
	rv = sqsh__superblock_builder_modification_time(&builder, 0);
	assert(rv == 0);
	rv = sqsh__superblock_builder_block_size(&builder, 32768);
	assert(rv == 0);
	rv = sqsh__superblock_builder_compression_id(&builder, compression_id);
	assert(rv == 0);
	rv = sqsh__superblock_builder_bytes_used(&builder, payload_size);
	assert(rv == 0);
	rv = sqsh__superblock_builder_id_table_start(&builder, ID_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_xattr_table_start(
			&builder, XATTR_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_inode_table_start(
			&builder, INODE_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_directory_table_start(
			&builder, DIRECTORY_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_fragment_table_start(
			&builder, FRAGMENT_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_export_table_start(
			&builder, EXPORT_TABLE_OFFSET);
	assert(rv == 0);
	rv = sqsh__superblock_builder_write(&builder, fsuperblock);
	assert(rv == sizeof(struct SqshDataSuperblock));
	rv = sqsh__superblock_builder_cleanup(&builder);
	assert(rv == 0);
	fclose(fsuperblock);

	const struct SqshConfig config = DEFAULT_CONFIG(payload_size);
	rv = sqsh__archive_init(sqsh, payload, &config);
	assert(0 == rv);
	(void)rv;
	return payload;
}
