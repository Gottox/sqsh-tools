/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "inode.h"
#include "error.h"
#include "inode_table.h"
#include "squash.h"
#include "stream.h"
#include "superblock.h"
#include <stdint.h>

static int
inode_data_more(struct SquashInode *inode, size_t size) {
	int rv = squash_stream_more(&inode->stream, size);

	if (rv < 0) {
		return rv;
	}
	inode->wrap = (struct SquashInodeWrap *)squash_stream_data(&inode->stream);
	return 0;
}

static int
inode_load(struct SquashInode *inode) {
	int rv = 0;
	size_t size = sizeof(struct SquashInodeHeader);

	switch (inode->wrap->header.inode_type) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		size += sizeof(struct SquashInodeDirectory);
		break;
	case SQUASH_INODE_TYPE_BASIC_FILE:
		size += sizeof(struct SquashInodeFileExt);
		break;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		size += sizeof(struct SquashInodeSymlink);
		break;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		size += sizeof(struct SquashInodeDevice);
		break;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		size += sizeof(struct SquashInodeIpc);
		break;
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		size += sizeof(struct SquashInodeDirectoryExt);
		break;
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		size += sizeof(struct SquashInodeFileExt);
		break;
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		size += sizeof(struct SquashInodeSymlinkExt);
		break;
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		size += sizeof(struct SquashInodeDeviceExt);
		break;
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		size += sizeof(struct SquashInodeIpcExt);
		break;
	}
	rv = inode_data_more(inode, size);
	return rv;
}

uint32_t
squash_inode_hard_link_count(struct SquashInode *inode) {
	struct SquashInodeWrap *wrap = inode->wrap;
	switch (wrap->header.inode_type) {
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
	return -SQUASH_ERROR_UNKOWN_INODE_TYPE;
}

int
squash_inode_load(struct SquashInode *inode, struct Squash *squash, uint64_t number) {
	int rv = 0;
	inode->wrap = NULL;
	int block = number >> 16;
	int offset = number & 0xffff;

	rv = squash_stream_init(
			&inode->stream, squash, &squash->inodes.metablock, block, offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, sizeof(struct SquashInodeHeader));
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);

	return rv;
}

int
squash_inode_cleanup(struct SquashInode *inode) {
	int rv = 0;
	rv = squash_stream_cleanup(&inode->stream);
	return rv;
}
