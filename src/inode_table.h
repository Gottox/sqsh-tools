/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode_table
 * @created     : Saturday May 08, 2021 21:20:54 CEST
 */

#include "metablock.h"

#ifndef INODE_TABLE_H

#define INODE_TABLE_H

struct Squash;

struct SquashInodeTable {
	struct SquashMetablock metablock;
};

int squash_inode_table_init(
		struct SquashInodeTable *table, struct Squash *squash);

int squash_inode_table_cleanup(struct SquashInodeTable *table);
#endif /* end of include guard INODE_TABLE_H */
