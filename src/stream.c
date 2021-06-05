/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : stream
 * @created     : Saturday May 08, 2021 21:43:15 CEST
 */

#include <stddef.h>
#include <stdint.h>

#include "compression/compression.h"
#include "error.h"
#include "metablock.h"
#include "squash.h"
#include "stream.h"

extern struct SquashDecompressorImpl squash_null_deflate;

static struct SquashDecompressor null_decompressor = {
		.info = {0},
		.compression_info_block = {0},
		.impl = &squash_null_deflate,
};

int
squash_stream_init(struct SquashStream *stream, struct Squash *squash,
		struct SquashMetablock *metablock, off_t block, off_t offset) {
	if (squash_metablock_is_compressed(metablock)) {
		stream->decompressor = &squash->decompressor;
	} else {
		stream->decompressor = &null_decompressor;
	}

	stream->offset = offset;
	stream->decompressor->impl->stream.init(&stream->decompressor->info,
			&stream->wrap, metablock->wrap->data,
			squash_metablock_size(metablock), block);

	return 0;
}

size_t
squash_stream_size(struct SquashStream *stream) {
	size_t plain_length =
			stream->decompressor->impl->stream.size(&stream->wrap);

	if (plain_length < stream->offset) {
		return 0;
	} else {
		return plain_length - stream->offset;
	}
}

int
squash_stream_more(struct SquashStream *stream, size_t min_read_bytes) {
	int rv = 0;

	while (squash_stream_size(stream) < min_read_bytes && rv == 0) {
		rv = stream->decompressor->impl->stream.decompress(&stream->wrap);
	}
	if (rv >= 0 && squash_stream_size(stream) < min_read_bytes) {
		rv = -SQUASH_ERROR_STREAM_NOT_ENOUGH_BYTES;
	}
	return rv;
}

int
squash_stream_to_end(struct SquashStream *stream) {
	int rv = 0;

	while (rv == 0) {
		rv = stream->decompressor->impl->stream.decompress(&stream->wrap);
	}
	return rv;
}

uint8_t *
squash_stream_data(struct SquashStream *stream) {
	uint8_t *data;

	if (squash_stream_size(stream) == 0) {
		return NULL;
	}

	data = stream->decompressor->impl->stream.data(&stream->wrap);
	return &data[stream->offset];
}

int
squash_stream_cleanup(struct SquashStream *stream) {
	stream->decompressor->impl->stream.cleanup(&stream->wrap);
	return 0;
}
