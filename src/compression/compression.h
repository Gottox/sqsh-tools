/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compressor
 * @created     : Friday Apr 30, 2021 19:33:51 CEST
 */

#include "../metablock.h"
#include "gzip_handler.h"

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef COMPRESSION_H

#define COMPRESSION_H

struct Squash;

struct SquashDecompressorImpl *squash_decompressor_by_id(int id);

union SquashDecompressorInfo {
	void *null;
	struct SquashGzip gzip;
};

struct SquashDecompressorImpl {
	int (*init)(union SquashDecompressorInfo *info, void *options, size_t size);
	int (*decompress)(union SquashDecompressorInfo *de, uint8_t **out,
			size_t *out_size, uint8_t *in, const off_t in_offset,
			const size_t in_size);
	int (*cleanup)(union SquashDecompressorInfo *de);
};

struct SquashDecompressor {
	union SquashDecompressorInfo info;
	struct SquashMetablock metablock;
	struct SquashDecompressorImpl *impl;
};

struct SquashDecompressorStream {
	struct SquashDecompressor *decompressor;
	off_t block_offset;
	off_t available_block_amount;

	uint8_t *data;
	size_t data_len;
};

int squash_decompressor_init(
		struct SquashDecompressor *de, struct Squash *squash);

int squash_decompressor_cleanup(struct SquashDecompressor *de);

#endif /* end of include guard COMPRESSION_H */
