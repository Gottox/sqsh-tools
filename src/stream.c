/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : stream
 * @created     : Saturday May 08, 2021 21:43:15 CEST
 */

#include "stream.h"
#include "metablock.h"
#include "squash.h"

int
squash_stream_init(struct SquashStream *stream, struct Squash *squash,
		struct SquashMetablock *metablock, off_t block, off_t offset) {
	stream->decompressor = &squash->decompressor;
	stream->metablock = metablock;
	stream->block = block;
	stream->offset = offset;

	stream->data = NULL;
	stream->data_len = 0;

	return 0;
}

size_t
squash_stream_size(struct SquashStream *stream) {
	if (stream->data_len < stream->offset)
		return 0;
	return stream->data_len - stream->offset;
}

int
squash_stream_more(struct SquashStream *stream, size_t min_read_bytes) {
	int rv = 0;
	struct SquashDecompressor *de = stream->decompressor;
	uint8_t *out = stream->data;
	off_t block_offset = stream->block;
	uint8_t *in = stream->metablock->wrap->data;

	while (squash_stream_size(stream) < min_read_bytes) {
		rv = de->impl->decompress_into(
				&de->info, &out, &block_offset, in, &min_read_bytes);
	}

	return 0;
}

uint8_t *
squash_stream_data(struct SquashStream *stream) {
	if (stream->data == NULL)
		return NULL;
	if (stream->data_len < stream->offset)
		return NULL;

	return &stream->data[stream->offset];
}

int
squash_stream_cleanup(struct SquashStream *stream) {
	return 0;
}
