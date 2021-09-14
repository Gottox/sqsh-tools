/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : inode
 * @created     : Thursday May 06, 2021 15:22:00 CEST
 */

#include "inode_context.h"
#include "../data/inode_internal.h"

#include "../data/superblock.h"
#include "../error.h"
#include "../extract.h"
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

	switch (squash_data_inode_type(inode->inode)) {
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
	switch (squash_data_inode_type(wrap)) {
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

uint64_t
squash_inode_file_size(struct SquashInodeContext *inode) {
	const struct SquashInodeFile *basic_file;
	const struct SquashInodeFileExt *extended_file;
	const struct SquashInodeDirectory *basic_dir;
	const struct SquashInodeDirectoryExt *extended_dir;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
		basic_dir = squash_data_inode_directory(inode->inode);
		return squash_data_inode_directory_file_size(basic_dir);
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		extended_dir = squash_data_inode_directory_ext(inode->inode);
		return squash_data_inode_directory_ext_file_size(extended_dir);
	case SQUASH_INODE_TYPE_BASIC_FILE:
		basic_file = squash_data_inode_file(inode->inode);
		return squash_data_inode_file_size(basic_file);
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		extended_file = squash_data_inode_file_ext(inode->inode);
		return squash_data_inode_file_ext_size(extended_file);
	}
	return 0;
}

uint16_t
squash_inode_permission(struct SquashInodeContext *inode) {
	return squash_data_inode_permissions(inode->inode);
}

uint32_t
squash_inode_modified_time(struct SquashInodeContext *inode) {
	return squash_data_inode_modified_time(inode->inode);
}

enum SquashInodeContextType
squash_inode_type(struct SquashInodeContext *inode) {
	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_DIRECTORY:
	case SQUASH_INODE_TYPE_EXTENDED_DIRECTORY:
		return SQUASH_INODE_TYPE_DIRECTORY;
	case SQUASH_INODE_TYPE_BASIC_FILE:
	case SQUASH_INODE_TYPE_EXTENDED_FILE:
		return SQUASH_INODE_TYPE_FILE;
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		return SQUASH_INODE_TYPE_SYMLINK;
	case SQUASH_INODE_TYPE_BASIC_BLOCK:
	case SQUASH_INODE_TYPE_EXTENDED_BLOCK:
		return SQUASH_INODE_TYPE_BLOCK;
	case SQUASH_INODE_TYPE_BASIC_CHAR:
	case SQUASH_INODE_TYPE_EXTENDED_CHAR:
		return SQUASH_INODE_TYPE_CHAR;
	case SQUASH_INODE_TYPE_BASIC_FIFO:
	case SQUASH_INODE_TYPE_EXTENDED_FIFO:
		return SQUASH_INODE_TYPE_FIFO;
	case SQUASH_INODE_TYPE_BASIC_SOCKET:
	case SQUASH_INODE_TYPE_EXTENDED_SOCKET:
		return SQUASH_INODE_TYPE_SOCKET;
	}
	return -SQUASH_INODE_TYPE_UNKNOWN;
}

const char *
squash_inode_symlink(struct SquashInodeContext *inode) {
	const struct SquashInodeSymlink *basic;
	const struct SquashInodeSymlinkExt *extended;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		basic = squash_data_inode_symlink(inode->inode);
		return (const char *)squash_data_inode_symlink_target_path(basic);
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = squash_data_inode_symlink_ext(inode->inode);
		return (const char *)squash_data_inode_symlink_ext_target_path(
				extended);
	}
	return NULL;
}

int
squash_inode_symlink_dup(struct SquashInodeContext *inode, char **namebuffer) {
	int size = squash_inode_symlink_size(inode);
	const char *link_target = squash_inode_symlink(inode);

	return squash_memdup(namebuffer, link_target, size);
}

uint32_t
squash_inode_symlink_size(struct SquashInodeContext *inode) {
	const struct SquashInodeSymlink *basic;
	const struct SquashInodeSymlinkExt *extended;

	switch (squash_data_inode_type(inode->inode)) {
	case SQUASH_INODE_TYPE_BASIC_SYMLINK:
		basic = squash_data_inode_symlink(inode->inode);
		return squash_data_inode_symlink_target_size(basic);
	case SQUASH_INODE_TYPE_EXTENDED_SYMLINK:
		extended = squash_data_inode_symlink_ext(inode->inode);
		return squash_data_inode_symlink_ext_target_size(extended);
	}
	return 0;
}

int
squash_inode_load_root(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, uint64_t inode_ref) {
	return squash_inode_load(inode, superblock,
			squash_data_superblock_root_inode_ref(superblock));
}

int
squash_inode_load(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, uint64_t inode_ref) {
	uint32_t inode_block;
	uint16_t inode_offset;

	squash_inode_ref_to_block(inode_ref, &inode_block, &inode_offset);

	int rv = 0;
	inode->inode = NULL;

	const struct SquashMetablock *metablock = squash_metablock_from_offset(
			superblock, squash_data_superblock_inode_table_start(superblock));
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

	if (squash_data_inode_type(inode->inode) !=
			SQUASH_INODE_TYPE_EXTENDED_DIRECTORY) {
		return SQUASH_ERROR_TODO;
	}

	const struct SquashInodeDirectoryExt *xdir =
			squash_data_inode_directory_ext(inode->inode);

	iterator->inode = inode;
	iterator->offset = sizeof(struct SquashInodeDirectoryExt);
	iterator->indices = squash_data_inode_directory_ext_index(xdir);
	iterator->remaining_entries =
			squash_data_inode_directory_ext_index_count(xdir);
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
	iterator->offset += squash_data_inode_directory_index_name_size(
			directory_index_by_offset(iterator, current_offset));
	rv = inode_data_more(iterator->inode, iterator->offset);
	if (rv < 0) {
		return NULL;
	}

	return current;
}
int
squash_inode_directory_index_iterator_clean(
		struct SquashInodeDirectoryIndexIterator *iterator) {
	return 0;
}

void
squash_inode_ref_to_block(
		uint64_t ref, uint32_t *block_index, uint16_t *offset) {
	*block_index = (ref & 0x0000FFFFFFFF0000) >> 16;
	*offset = ref & 0x000000000000FFFF;
}
uint64_t
squash_inode_ref_from_block(uint32_t block_index, uint16_t offset) {
	return ((uint64_t)block_index << 16) | offset;
}
