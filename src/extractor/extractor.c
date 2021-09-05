/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : extractor
 * @created     : Sunday Sep 05, 2021 12:16:28 CEST
 */

#include "extractor.h"
#include "../error.h"
#include "../squash.h"
#include "../format/metablock.h"
#include "gzip.h"
#include "null.h"

static const struct SquashExtractorImplementation *
extractor_by_id(int id) {
	switch ((enum SquashSuperblockCompressionId)id) {
	case SQUASH_COMPRESSION_NONE:
		return &squash_extractor_null;
	case SQUASH_COMPRESSION_GZIP:
		return &squash_extractor_gzip;
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
squash_extractor_init(struct Squash *squash,
		struct SquashExtractor *extractor) {
	int rv = 0;
	const struct SquashExtractorImplementation *impl;

	impl = extractor_by_id(squash->superblock.wrap->compression_id);
	if (impl == NULL) {
		return -SQUASH_ERROR_COMPRESSION_INIT;
	}

	const struct SquashMetablock *metablock = squash_metablock_from_offset(squash, sizeof(struct SquashSuperblockWrap));
	if (metablock == NULL) {
		return -SQUASH_ERROR_TODO;
	}

	rv = impl->init(&extractor->options, squash_metablock_data(metablock), squash_metablock_size(metablock));
	if (rv < 0) {
		return rv;
	}

	return rv;
}

int squash_extractor_extract(struct SquashExtractor *extractor,
		uint8_t **target, size_t *target_size, const uint8_t *compressed,
		const size_t compressed_size) {
	const struct SquashExtractorImplementation *impl = extractor->impl;
	union SquashExtractorOptions *options = &extractor->options;

	return impl->extract(options, target, target_size, compressed, compressed_size);
}

int squash_extractor_cleanup(struct SquashExtractor *extractor) {
	const struct SquashExtractorImplementation *impl = extractor->impl;
	union SquashExtractorOptions *options = &extractor->options;

	return impl->cleanup(options);
}
