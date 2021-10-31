/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : xattr_table_context
 * @created     : Sunday Oct 31, 2021 11:54:43 CET
 */

#include "../utils.h"
#include "metablock_context.h"
#include "table_context.h"
#include <stdint.h>

#ifndef XATTR_TABLE_CONTEXT_H

#define XATTR_TABLE_CONTEXT_H

struct SquashSuperblockContext;
struct SquashXattrKey;
struct SquashXattrValue;
struct SquashInodeContext;

struct SquashXattrTableContext {
	struct SquashSuperblockContext *superblock;
	const struct SquashXattrIdTable *header;
	struct SquashTableContext table;
};

struct SquashXattrTableIterator {
	struct SquashMetablockContext metablock;
	struct SquashMetablockContext out_of_line_value;
	struct SquashXattrTableContext *context;
	int remaining_entries;
	struct SquashXattrKey *current_key;
	struct SquashXattrValue *current_value;
};

SQUASH_NO_UNUSED int squash_xattr_table_init(
		struct SquashXattrTableContext *context,
		struct SquashSuperblockContext *superblock);

SQUASH_NO_UNUSED int squash_xattr_table_iterator_init(
		struct SquashXattrTableIterator *iterator,
		struct SquashXattrTableContext *xattr_table,
		const struct SquashInodeContext *inode);

int squash_xattr_table_iterator_next(struct SquashXattrTableIterator *iterator);

uint16_t
squash_xattr_table_iterator_type(struct SquashXattrTableIterator *iterator);

bool squash_xattr_table_iterator_is_indirect(
		struct SquashXattrTableIterator *iterator);

const char *
squash_xattr_table_iterator_name(struct SquashXattrTableIterator *iterator);
int squash_xattr_table_iterator_fullname_dup(
		struct SquashXattrTableIterator *iterator, char **fullname_buffer);

uint16_t squash_xattr_table_iterator_name_size(
		struct SquashXattrTableIterator *iterator);

int squash_xattr_table_iterator_value_dup(
		struct SquashXattrTableIterator *iterator, char **value_buffer);

const char *
squash_xattr_table_iterator_value(struct SquashXattrTableIterator *iterator);

uint16_t squash_xattr_table_iterator_value_size(
		struct SquashXattrTableIterator *iterator);

int
squash_xattr_table_iterator_cleanup(struct SquashXattrTableIterator *iterator);

int squash_xattr_table_cleanup(struct SquashXattrTableContext *context);

#endif /* end of include guard XATTR_TABLE_CONTEXT_H */
