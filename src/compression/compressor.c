/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compressor
 * @created     : Wednesday Dec 22, 2021 18:28:00 CET
 */

#include "compressor.h"
#include "../context/superblock_context.h"
#include "../error.h"
#include "compression.h"

#ifdef CONFIG_ZLIB
extern const struct HsqsCompressionImplementation hsqs_compression_zlib;
#endif
#ifdef CONFIG_LZMA
extern const struct HsqsCompressionImplementation hsqs_compression_lzma;
extern const struct HsqsCompressionImplementation hsqs_compression_xz;
#endif
#ifdef CONFIG_LZO
extern const struct HsqsCompressionImplementation hsqs_compression_lzo;
#endif
#ifdef CONFIG_LZ4
extern const struct HsqsCompressionImplementation hsqs_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
extern const struct HsqsCompressionImplementation hsqs_compression_zstd;
#endif
extern const struct HsqsCompressionImplementation hsqs_compression_null;

static const struct HsqsCompressionImplementation *
compression_by_id(int id) {
	switch ((enum HsqsSuperblockCompressionId)id) {
	case HSQS_COMPRESSION_NONE:
		return &hsqs_compression_null;
#ifdef CONFIG_ZLIB
	case HSQS_COMPRESSION_GZIP:
		return &hsqs_compression_zlib;
#endif
#ifdef CONFIG_LZMA
	case HSQS_COMPRESSION_LZMA:
		return &hsqs_compression_lzma;
#endif
#ifdef CONFIG_XZ
	case HSQS_COMPRESSION_XZ:
		return &hsqs_compression_xz;
#endif
#ifdef CONFIG_LZO
	case HSQS_COMPRESSION_LZO:
		return &hsqs_compression_lzo;
#endif
#ifdef CONFIG_LZ4
	case HSQS_COMPRESSION_LZ4:
		return &hsqs_compression_lz4;
#endif
#ifdef CONFIG_ZSTD
	case HSQS_COMPRESSION_ZSTD:
		return &hsqs_compression_zstd;
#endif
	default:
		return NULL;
	}
}

int
hsqs_compressor_init(struct HsqsCompressor *compressor, int compressor_id) {
	const struct HsqsCompressionImplementation *impl =
			compression_by_id(compressor_id);
	if (impl == NULL) {
		return -HSQS_ERROR_COMPRESSION_UNKNOWN;
	}
	compressor->impl = impl;

	return 0;
}

int
hsqs_compressor_write_to_buffer(
		struct HsqsCompressor *compressor, const uint8_t *source,
		const size_t source_size, struct HsqsBuffer *buffer) {
	const union HsqsCompressionOptions *options = NULL;
	size_t options_size = 0;
	int rv = 0;
	size_t block_size = buffer->block_size;
	size_t old_size = buffer->size;
	size_t new_size;

	if (ADD_OVERFLOW(old_size, block_size, &new_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}

	rv = hsqs_buffer_set_size(buffer, new_size);
	if (rv < 0) {
		return rv;
	}
	rv = compressor->impl->extract(
			options, options_size, &buffer->data[old_size], &block_size, source,
			source_size);
	if (rv < 0)
		return rv;

	// extract mutates the block_size and sets it to the
	// actual size of the decompressed block. So we need
	// to recalculate the new size below
	if (ADD_OVERFLOW(old_size, block_size, &new_size)) {
		return -HSQS_ERROR_INTEGER_OVERFLOW;
	}
	rv = hsqs_buffer_set_size(buffer, new_size);

	return rv;
}

int
hsqs_compressor_cleanup(struct HsqsCompressor *compressor) {
	(void)compressor;
	return 0;
}
