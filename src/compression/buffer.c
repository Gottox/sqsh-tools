/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 12:16:28 CEST
 */

#include "../context/metablock_context.h"
#include "../data/metablock.h"
#include "../data/superblock_internal.h"
#include "../error.h"
#include "compression.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef CONFIG_COMPRESSION_GZIP
extern const struct SquashCompressionImplementation squash_compression_gzip;
#endif
#ifdef CONFIG_COMPRESSION_LZMA
extern const struct SquashCompressionImplementation squash_compression_lzma;
#endif
#ifdef CONFIG_COMPRESSION_XZ
extern const struct SquashCompressionImplementation squash_compression_xz;
#endif
#ifdef CONFIG_COMPRESSION_LZO
extern const struct SquashCompressionImplementation squash_compression_lzo;
#endif
#ifdef CONFIG_COMPRESSION_LZ4
extern const struct SquashCompressionImplementation squash_compression_lz4;
#endif
#ifdef CONFIG_COMPRESSION_ZSTD
extern const struct SquashCompressionImplementation squash_compression_zstd;
#endif
extern const struct SquashCompressionImplementation squash_compression_null;

static const struct SquashCompressionImplementation *
compression_by_id(int id) {
	switch ((enum SquashSuperblockCompressionId)id) {
	case SQUASH_COMPRESSION_NONE:
		return &squash_compression_null;
#ifdef CONFIG_COMPRESSION_GZIP
	case SQUASH_COMPRESSION_GZIP:
		return &squash_compression_gzip;
#endif
#ifdef CONFIG_COMPRESSION_LZMA
	case SQUASH_COMPRESSION_LZMA:
		return &squash_compression_lzma;
#endif
#ifdef CONFIG_COMPRESSION_XZ
	case SQUASH_COMPRESSION_XZ:
		return &squash_compression_xz;
#endif
#ifdef CONFIG_COMPRESSION_LZO
	case SQUASH_COMPRESSION_LZO:
		return &squash_compression_lzo;
#endif
#ifdef CONFIG_COMPRESSION_LZ4
	case SQUASH_COMPRESSION_LZ4:
		return &squash_compression_lz4;
#endif
#ifdef CONFIG_COMPRESSION_ZSTD
	case SQUASH_COMPRESSION_ZSTD:
		return &squash_compression_zstd;
#endif
	default:
		return NULL;
	}
}

int
squash_buffer_init(struct SquashBuffer *buffer,
		const struct SquashSuperblock *superblock, int block_size) {
	int rv = 0;
	const struct SquashCompressionImplementation *impl;

	impl = compression_by_id(squash_data_superblock_compression_id(superblock));
	if (impl == NULL) {
		return -SQUASH_ERROR_COMPRESSION_INIT;
	}

	if (squash_data_superblock_flags(superblock) &
			SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		// TODO: manually calculating the offset does not honour bounds checks
		const struct SquashMetablock *metablock =
				(const struct SquashMetablock *)&((const uint8_t *)
								superblock)[sizeof(struct SquashSuperblock)];
		buffer->options = (const union SquashCompressionOptions *)
				squash_data_metablock_data(metablock);
	} else {
		buffer->options = NULL;
	}

	if (rv < 0) {
		return rv;
	}
	buffer->impl = impl;
	buffer->block_size = block_size;

	return rv;
}

int
squash_buffer_append(struct SquashBuffer *buffer, const uint8_t *source,
		const size_t source_size, bool is_compressed) {
	const union SquashCompressionOptions *options = buffer->options;
	const struct SquashCompressionImplementation *impl =
			is_compressed ? buffer->impl : &squash_compression_null;
	int rv = 0;
	size_t block_size = buffer->block_size;
	const size_t buffer_size = buffer->size;

	buffer->data = realloc(buffer->data, buffer_size + block_size);
	if (buffer->data == NULL) {
		return -SQUASH_ERROR_MALLOC_FAILED;
	}

	rv = impl->extract(options, &buffer->data[buffer_size], &block_size, source,
			source_size);
	if (rv < 0)
		return rv;

	buffer->size += block_size;

	return rv;
}

const uint8_t *
squash_buffer_data(const struct SquashBuffer *buffer) {
	return buffer->data;
}
size_t
squash_buffer_size(const struct SquashBuffer *buffer) {
	return buffer->size;
}

int
squash_buffer_cleanup(struct SquashBuffer *buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->size = 0;
	return 0;
}
