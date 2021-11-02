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

struct HsqsSuperblockContext;
struct HsqsXattrKey;
struct HsqsXattrValue;
struct HsqsInodeContext;

struct HsqsXattrTableContext {
	struct HsqsSuperblockContext *superblock;
	const struct HsqsXattrIdTable *header;
	struct HsqsTableContext table;
};

struct HsqsXattrTableIterator {
	struct HsqsMetablockContext metablock;
	struct HsqsMetablockContext out_of_line_value;
	struct HsqsXattrTableContext *context;
	int remaining_entries;
	struct HsqsXattrKey *current_key;
	struct HsqsXattrValue *current_value;
};

HSQS_NO_UNUSED int hsqs_xattr_table_init(
		struct HsqsXattrTableContext *context,
		struct HsqsSuperblockContext *superblock);

HSQS_NO_UNUSED int hsqs_xattr_table_iterator_init(
		struct HsqsXattrTableIterator *iterator,
		struct HsqsXattrTableContext *xattr_table,
		const struct HsqsInodeContext *inode);

int hsqs_xattr_table_iterator_next(struct HsqsXattrTableIterator *iterator);

uint16_t
hsqs_xattr_table_iterator_type(struct HsqsXattrTableIterator *iterator);

bool
hsqs_xattr_table_iterator_is_indirect(struct HsqsXattrTableIterator *iterator);

const char *
hsqs_xattr_table_iterator_prefix(struct HsqsXattrTableIterator *iterator);
uint16_t
hsqs_xattr_table_iterator_prefix_size(struct HsqsXattrTableIterator *iterator);
const char *
hsqs_xattr_table_iterator_name(struct HsqsXattrTableIterator *iterator);
uint16_t
hsqs_xattr_table_iterator_name_size(struct HsqsXattrTableIterator *iterator);
int hsqs_xattr_table_iterator_fullname_dup(
		struct HsqsXattrTableIterator *iterator, char **fullname_buffer);

int hsqs_xattr_table_iterator_value_dup(
		struct HsqsXattrTableIterator *iterator, char **value_buffer);

const char *
hsqs_xattr_table_iterator_value(struct HsqsXattrTableIterator *iterator);

uint16_t
hsqs_xattr_table_iterator_value_size(struct HsqsXattrTableIterator *iterator);

int hsqs_xattr_table_iterator_cleanup(struct HsqsXattrTableIterator *iterator);

int hsqs_xattr_table_cleanup(struct HsqsXattrTableContext *context);

#endif /* end of include guard XATTR_TABLE_CONTEXT_H */
