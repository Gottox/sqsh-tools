/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 12:16:28 CEST
 */

#include "compression.h"
#include "../error.h"
#include "../format/metablock.h"
#include "../squash.h"

static const struct SquashCompressionImplementation *
compression_by_id(int id) {
	switch ((enum SquashSuperblockCompressionId)id) {
	case SQUASH_COMPRESSION_NONE:
		return &squash_compression_null;
	case SQUASH_COMPRESSION_GZIP:
		return &squash_compression_gzip;
	case SQUASH_COMPRESSION_LZMA:
		return &squash_compression_lzma;
	case SQUASH_COMPRESSION_XZ:
		return &squash_compression_xz;
	case SQUASH_COMPRESSION_LZO:
		return &squash_compression_lzo;
	case SQUASH_COMPRESSION_LZ4:
		return &squash_compression_lz4;
	case SQUASH_COMPRESSION_ZSTD:
		return &squash_compression_zstd;
	default:
		return NULL;
	}
}

int
squash_compression_init(
		struct Squash *squash, struct SquashCompression *compression) {
	int rv = 0;
	const struct SquashCompressionImplementation *impl;

	impl = compression_by_id(
			squash_superblock_compression_id(squash->superblock));
	if (impl == NULL) {
		return -SQUASH_ERROR_COMPRESSION_INIT;
	}

	if (squash_superblock_flags(squash->superblock) &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		const struct SquashMetablock *metablock =
				squash_metablock_from_offset(squash, SQUASH_SUPERBLOCK_SIZE);
		if (metablock == NULL) {
			return -SQUASH_ERROR_TODO;
		}
		compression->options =
				(const union SquashCompressionOptions *)squash_metablock_data(
						metablock);
	} else {
		compression->options = impl->default_options;
	}

	if (rv < 0) {
		return rv;
	}
	compression->impl = impl;

	return rv;
}

int
squash_compression_extract(struct SquashCompression *compression,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	const struct SquashCompressionImplementation *impl = compression->impl;
	const union SquashCompressionOptions *options = compression->options;

	return impl->extract(
			options, target, target_size, compressed, compressed_size);
}

int
squash_compression_cleanup(struct SquashCompression *extractor) {
	return 0;
}
