/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : file_content_context
 * @created     : Tuesday Sep 14, 2021 22:56:07 CEST
 */

#include "file_content_context.h"
#include "../data/metablock.h"
#include "../data/superblock.h"
#include "../error.h"

#include <stdint.h>

int
squash_file_content_init(struct SquashFileContentContext *file_content,
		const struct SquashSuperblock *superblock,
		struct SquashInodeContext *inode) {
	size_t block_size = squash_data_superblock_block_size(superblock);
	int rv = 0;

	if (squash_inode_type(inode) != SQUASH_INODE_TYPE_FILE) {
		return -SQUASH_ERROR_NOT_A_FILE;
	}
	file_content->superblock = superblock;
	file_content->inode = inode;
	file_content->blocks =
			(const uint8_t *)superblock + squash_inode_file_blocks_start(inode);
	file_content->blocks_count = squash_inode_file_size(inode) /
			squash_data_superblock_block_size(superblock);

	rv = squash_buffer_init(&file_content->buffer, superblock, block_size);
	if (rv < 0) {
		return rv;
	}
	return rv;
}

static uint32_t
datablock_size(struct SquashFileContentContext *file_content, uint64_t index) {
	struct SquashInodeContext *inode = file_content->inode;

	return squash_inode_file_block_size(inode, index) & ~(1 << 24);
}

static bool
datablock_is_compressed(
		struct SquashFileContentContext *file_content, uint64_t index) {
	struct SquashInodeContext *inode = file_content->inode;

	return 0 == (squash_inode_file_block_size(inode, index) & (1 << 24));
}

static const uint64_t
datablock_offset(
		struct SquashFileContentContext *file_content, uint64_t index) {
	off_t offset = 0;

	for (int i = 0; i < index; i++) {
		offset += datablock_size(file_content, i);
	}

	return offset;
}

int
squash_file_content_seek(
		struct SquashFileContentContext *context, uint64_t seek_pos) {
	size_t block_size = squash_data_superblock_block_size(context->superblock);

	context->datablock_index = seek_pos / block_size;
	context->datablock_offset = seek_pos % block_size;

	return 0;
}

void *
squash_file_content_data(const struct SquashFileContentContext *context) {
	return &context->buffer.data[context->datablock_offset];
}

size_t
squash_file_content_size(const struct SquashFileContentContext *context) {
	const size_t buffer_size = squash_buffer_size(&context->buffer);
	if (context->datablock_offset > buffer_size) {
		return 0;
	} else {
		return buffer_size - context->datablock_offset;
	}
}

int
squash_file_content_read(
		struct SquashFileContentContext *context, uint64_t size) {
	int rv = 0;

	uint64_t compressed_offset =
			datablock_offset(context, context->datablock_index);

	while (rv == 0 && squash_file_content_size(context) < size) {
		bool is_compressed =
				datablock_is_compressed(context, compressed_offset);
		uint64_t compressed_size = datablock_size(context, compressed_offset);
		rv = squash_buffer_append(&context->buffer,
				&context->blocks[compressed_offset], compressed_size,
				is_compressed);
		compressed_offset += compressed_size;
		context->datablock_index++;
	}

	return rv;
}

int
squash_file_content_clean(struct SquashFileContentContext *file_content) {
	return squash_buffer_cleanup(&file_content->buffer);
}
