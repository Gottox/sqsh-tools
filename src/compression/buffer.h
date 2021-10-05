/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression
 * @created     : Sunday Sep 05, 2021 10:50:12 CEST
 */

#include "../utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef EXTRACTOR_H

#define EXTRACTOR_H

struct SquashSuperblock;

struct SquashBuffer {
	const union SquashCompressionOptions *options;
	const struct SquashCompressionImplementation *impl;
	int block_size;
	uint8_t *data;
	size_t size;
};

SQUASH_NO_UNUSED int squash_buffer_init(struct SquashBuffer *compression,
		const struct SquashSuperblock *superblock, int block_size);

SQUASH_NO_UNUSED int squash_buffer_append(struct SquashBuffer *compression,
		const uint8_t *source, const size_t source_size, bool is_compressed);

const uint8_t *squash_buffer_data(const struct SquashBuffer *buffer);
size_t squash_buffer_size(const struct SquashBuffer *buffer);

int squash_buffer_cleanup(struct SquashBuffer *compression);

#endif /* end of include guard EXTRACTOR_H */
