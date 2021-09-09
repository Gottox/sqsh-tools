/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "inode_context.h"
#include "../format/inode_internal.h"

#include "../error.h"
#include "../extract.h"
#include "../format/superblock.h"
#include "../squash.h"
#include "../utils.h"
#include "metablock_context.h"
#include <stdint.h>

static struct SquashInodeDirectoryIndex *
directory_index_by_offset(
		struct SquashInodeDirectoryIndexIterator *iterator, off_t offset) {
	const uint8_t *tmp = (const uint8_t *)iterator->inode->inode;
	return (struct SquashInodeDirectoryIndex *)&tmp[offset];
}

static int
inode_data_more(struct SquashInodeContext *inode, size_t size) {
	int rv = squash_extract_more(&inode->extract, size);

	if (rv < 0) {
		return rv;
	}
	inode->inode = (struct SquashInode *)squash_extract_data(&inode->extract);
	return rv;
}

static int
inode_load(struct SquashInodeContext *inode) {
	int rv = 0;
	size_t size = sizeof(struct SquashInodeHeader);

	switch (squash_format_inode_type(inode->inode)) {
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
squash_inode_hard_link_count(struct SquashInodeContext *inode) {
	struct SquashInode *wrap = inode->inode;
	switch (squash_format_inode_type(wrap)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		return wrap->data.directory.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_FILE:
		return 1;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		return wrap->data.symlink.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_BASIC_CHAR:
		return wrap->data.device_ext.hard_link_count;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
		return wrap->data.ipc.hard_link_count;

	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		return wrap->data.directory_ext.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		return wrap->data.file_ext.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		return wrap->data.symlink_ext.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		return wrap->data.device_ext.hard_link_count;
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		return wrap->data.ipc_ext.hard_link_count;
	}
	return -SQUASH_ERROR_UNKOWN_INODE_TYPE;
}

int
squash_inode_load(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, int inode_block,
		int inode_offset) {
	int rv = 0;
	inode->inode = NULL;

	const struct SquashMetablock *metablock = squash_metablock_from_offset(
			superblock, squash_superblock_inode_table_start(superblock));
	if (metablock == NULL) {
		return -SQUASH_ERROR_INODE_INIT;
	}
	rv = squash_extract_init(
			&inode->extract, superblock, metablock, inode_block, inode_offset);
	if (rv < 0) {
		return rv;
	}

	// loading enough data to identify the inode
	rv = inode_data_more(inode, sizeof(struct SquashInodeHeader));
	if (rv < 0) {
		return rv;
	}

	rv = inode_load(inode);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

int
squash_inode_load_ref(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, uint64_t inode_ref) {
	int block = (inode_ref & 0x0000FFFFFFFF0000) >> 16;
	int offset = inode_ref & 0x000000000000FFFF;

	return squash_inode_load(inode, superblock, block, offset);
}

int
squash_inode_cleanup(struct SquashInodeContext *inode) {
	int rv = 0;
	rv = squash_extract_cleanup(&inode->extract);
	return rv;
}

int
squash_inode_directory_iterator_init(
		struct SquashInodeDirectoryIndexIterator *iterator,
		struct SquashInodeContext *inode) {
	int rv = 0;
	const struct SquashInodeDirectoryExt *xdir =
			squash_format_inode_directory_ext(inode->inode);

	iterator->inode = inode;
	iterator->offset = sizeof(struct SquashInodeDirectoryExt);
	iterator->indices = squash_format_inode_directory_ext_index(xdir);
	iterator->remaining_entries =
			squash_format_inode_directory_ext_index_count(xdir);
	return rv;
}
const struct SquashInodeDirectoryIndex *
squash_inode_directory_index_iterator_next(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	off_t current_offset = iterator->offset;
	// Make sure next entry is loaded:
	iterator->offset += sizeof(struct SquashInodeDirectoryIndex);
	rv = inode_data_more(iterator->inode, iterator->offset);
	if (rv < 0) {
		return NULL;
	}

	const struct SquashInodeDirectoryIndex *current =
			directory_index_by_offset(iterator, current_offset);
	// Make sure current index has its name populated
	iterator->offset += squash_format_inode_directory_index_name_size(
			directory_index_by_offset(iterator, current_offset));
	rv = inode_data_more(iterator->inode, iterator->offset);
	if (rv < 0) {
		return NULL;
	}

	return current;
}
int
squash_inode_directory_iterator_clean(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	return 0;
}
