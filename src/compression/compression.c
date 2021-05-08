/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Friday May 07, 2021 11:15:34 CEST
 */

#include "compression.h"
#include "../metablock.h"
#include "../squash.h"
#include "../superblock.h"
#include <stdint.h>

extern struct SquashDecompressorImpl squash_null_deflate;
extern struct SquashDecompressorImpl squash_gzip_deflate;

struct SquashDecompressorImpl *
squash_decompressor_by_id(int id) {
	switch ((enum SquashSuperblockCompressionId)id) {
	case SQUASH_COMPRESSION_NONE:
		return &squash_null_deflate;
	case SQUASH_COMPRESSION_GZIP:
		return &squash_gzip_deflate;
	case SQUASH_COMPRESSION_LZMA:
	case SQUASH_COMPRESSION_LZO:
	case SQUASH_COMPRESSION_XZ:
	case SQUASH_COMPRESSION_LZ4:
	case SQUASH_COMPRESSION_ZSTD:
		return NULL;
	}
	return NULL;
}

int
squash_decompressor_init(struct SquashDecompressor *de, struct Squash *squash) {
	int rv = 0;
	if (squash->superblock->flags & SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		rv = squash_metablock_init(
				&de->metablock, squash, SQUASH_SUPERBLOCK_SIZE);
		if (rv < 0) {
			goto err;
		}
	}

	de->impl = squash_decompressor_by_id(squash->superblock->compression_id);
	if (de->impl == NULL) {
		rv = -SQUASH_ERROR_UNSUPPORTED_COMPRESSION;
	}
	uint8_t *data = squash_metablock_data(&de->metablock);
	size_t size = squash_metablock_size(&de->metablock);
	rv = de->impl->init(&de->info, data, size);
	if (rv < 0) {
		goto err;
	}

	return 0;
err:
	squash_decompressor_cleanup(de);
	return rv;
}

int
squash_decompressor_cleanup(struct SquashDecompressor *de) {
	if (de == NULL)
		return 0;

	if (de->impl) {
		de->impl->cleanup(&de->info);
	}
	squash_metablock_cleanup(&de->metablock);
	return 0;
}

int
squash_decompressor_stream_init(struct SquashDecompressorStream *stream,
		struct SquashDecompressor *de, off_t block_offset) {
	stream->decompressor = de;
	stream->block_offset = block_offset;
	stream->available_block_amount = 0;

	stream->data = NULL;
	stream->data_len = 0;

	return 0;
}

int
squash_decompressor_stream_more(
		struct SquashDecompressorStream *stream, size_t min_read_bytes) {
	return 0;
}

int
squash_decompressor_stream_clean(
		struct SquashDecompressorStream *stream, size_t min_read_bytes) {
	int rv = 0;
	return rv;
}
