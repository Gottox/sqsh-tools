/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode_table
 * @created     : Saturday May 08, 2021 21:21:01 CEST
 */

#include <stdlib.h>

#include "inode.h"
#include "inode_table.h"
#include "metablock.h"
#include "squash.h"
#include "superblock.h"

int
squash_inode_table_init(struct SquashInodeTable *table, struct Squash *squash) {
	int rv = 0;
	rv = squash_metablock_init(
			table->metablock, squash, squash->superblock->inode_table_start);

	return 0;
}

int
squash_inode_table_cleanup(struct SquashInodeTable *table) {
	int rv = 0;

	rv = squash_metablock_cleanup(table->metablock);
	return rv;
}
