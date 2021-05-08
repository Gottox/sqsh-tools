/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : null
 * @created     : Friday Apr 30, 2021 20:49:40 CEST
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"

int
squash_null_init(union SquashDecompressorInfo *info, void *options, size_t size) {
	return 0;
}

int
squash_null_decompress(union SquashDecompressorInfo *de, uint8_t **out,
		size_t *out_size, uint8_t *in, const off_t in_offset,
		const size_t in_size) {
	*out_size = in_size;
	*out = in;

	return in_size;
}

int
squash_null_cleanup(union SquashDecompressorInfo *de) {
	return 0;
}

struct SquashDecompressorImpl squash_null_deflate = {
	.init = squash_null_init,
	.decompress = squash_null_decompress,
	.cleanup = squash_null_cleanup,
};
