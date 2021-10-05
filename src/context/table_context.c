/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : table_context
 * @created     : Sunday Sep 26, 2021 19:11:49 CEST
 */

#include "table_context.h"
#include "../error.h"
#include <stdint.h>

int
squash_table_init(struct SquashTableContext *table,
		const struct SquashSuperblock *superblock, off_t start_block,
		size_t element_size, size_t element_count) {
	int rv = 0;
	size_t byte_size;

	table->lookup_table = (uint64_t *)&((uint8_t *)superblock)[start_block];

	rv = squash_metablock_init(
			&table->metablock, superblock, table->lookup_table[0]);
	if (rv < 0) {
		goto out;
	}

	table->element_size = element_size;
	table->element_count = element_count;
	if (MULT_OVERFLOW(element_size, element_count, &byte_size)) {
		rv = -SQUASH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	rv = squash_metablock_more(&table->metablock, byte_size);

out:
	return rv;
}

int
squash_table_get(
		struct SquashTableContext *table, off_t index, const void **target) {
	off_t offset;

	if (MULT_OVERFLOW(index, table->element_size, &offset)) {
		return -SQUASH_ERROR_INTEGER_OVERFLOW;
	}
	*target = &squash_metablock_data(&table->metablock)[offset];

	return 0;
}

int
squash_table_cleanup(struct SquashTableContext *table) {
	squash_metablock_cleanup(&table->metablock);
	return 0;
}
