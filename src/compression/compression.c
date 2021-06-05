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

static struct SquashDecompressorImpl *
decompressor_by_id(int id) {
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
	default:
		return NULL;
	}
}

int
squash_decompressor_init(struct SquashDecompressor *de, struct Squash *squash) {
	int rv = 0;
	uint8_t *compression_info = NULL;
	size_t compression_info_size = 0;

	if (squash->superblock->flags & SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		rv = squash_metablock_init(
				&de->compression_info_block, squash, SQUASH_SUPERBLOCK_SIZE);
		if (rv < 0) {
			goto err;
		}
		if (squash_metablock_is_compressed(&de->compression_info_block)) {
			rv = -SQUASH_ERROR_METABLOCK_INFO_IS_COMPRESSED;
			goto err;
		}
		compression_info = squash_metablock_data(&de->compression_info_block);
		compression_info_size =
				squash_metablock_size(&de->compression_info_block);
	}

	de->impl = decompressor_by_id(squash->superblock->compression_id);
	if (de->impl == NULL) {
		rv = -SQUASH_ERROR_METABLOCK_UNSUPPORTED_COMPRESSION;
		goto err;
	}
	rv = de->impl->init(&de->info, compression_info, compression_info_size);
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
	squash_metablock_cleanup(&de->compression_info_block);
	return 0;
}
