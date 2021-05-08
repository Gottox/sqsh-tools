/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "inode.h"

uint32_t
squash_inode_hard_link_count(struct SquashInode *inode) {
	struct SquashInodeWrap *wrap = inode->wrap;
	switch (wrap->inode_type) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		return wrap->data.dir.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_FILE:
		return 1;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		return wrap->data.sym.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		return wrap->data.dev.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		return wrap->data.ipc.hard_link_count;

	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		return wrap->data.xdir.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		return wrap->data.xfile.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		return wrap->data.xsym.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		return wrap->data.xdev.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		return wrap->data.xipc.hard_link_count;
	}
	return -1;
}
