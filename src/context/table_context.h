/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : table_context
 * @created     : Sunday Sep 26, 2021 19:04:39 CEST
 */

#include "metablock_context.h"
#include <stdint.h>

#ifndef TABLE_CONTEXT_H

#define TABLE_CONTEXT_H

struct SquashTableContext {
	struct SquashMetablockContext metablock;
	uint64_t *lookup_table;
	size_t element_size;
	size_t element_count;
};

int squash_table_init(struct SquashTableContext *table,
		const struct SquashSuperblock *superblock, off_t start_block,
		size_t element_size, size_t element_count);
int squash_table_get(
		struct SquashTableContext *table, off_t index, const void **target);
int squash_table_cleanup(struct SquashTableContext *table);

#endif /* end of include guard TABLE_CONTEXT_H */
