/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : util
 * @created     : Saturday Feb 18, 2023 14:06:14 CET
 */

// for fmemopen
#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include "../include/sqsh_archive_private.h"
#include "../include/sqsh_data_private.h"
#include "../include/sqsh_mapper.h"
#include "../lib/build/sqsh_archive_builder.h"
#include "../lib/build/sqsh_file_builder.h"

#include <assert.h>

void
mk_symlink(
		char *target, uint32_t inode, sqsh_index_t offset, uint8_t *payload,
		size_t payload_size) {
	int rv;
	struct SqshInodeBuilder builder = {0};
	FILE *archive = fmemopen(payload, payload_size, "r+");
	assert(archive);
	rv = fseek(archive, offset, SEEK_SET);
	assert(rv == 0);
	rv = sqsh__inode_builder_init(&builder);
	assert(rv == 0);
	rv = sqsh__inode_builder_inode_number(&builder, inode);
	assert(rv == 0);
	rv = sqsh__inode_builder_symlink(&builder, target);
	assert(rv == 0);
	rv = sqsh__inode_builder_write(&builder, archive);
	// TODO: Calculate actual size;
	assert(rv > 0);
	rv = sqsh__inode_builder_cleanup(&builder);
	fclose(archive);
}

const uint8_t *
mk_stub(struct SqshArchive *sqsh, uint8_t *payload, size_t payload_size) {
	int rv;
	const int compression_id =
			payload[20] ? payload[20] : SQSH_COMPRESSION_GZIP;
	struct SqshSuperblockBuilder builder = {0};
	FILE *archive = fmemopen(payload, payload_size, "r+");
	assert(archive);
	rv = sqsh__superblock_builder_init(&builder);
	assert(rv == 0);
	rv = sqsh__superblock_builder_inode_count(&builder, 100);
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
	rv = sqsh__superblock_builder_write(&builder, archive);
	assert(rv == sizeof(struct SqshDataSuperblock));
	rv = sqsh__superblock_builder_cleanup(&builder);
	assert(rv == 0);
	fclose(archive);

	const struct SqshConfig config = DEFAULT_CONFIG(payload_size);
	rv = sqsh__archive_init(sqsh, payload, &config);
	assert(0 == rv);
	(void)rv;
	return payload;
}
