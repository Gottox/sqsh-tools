/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory_index_iterator
 * @created     : Monday Dec 20, 2021 12:41:47 CET
 */

#include "directory_index_iterator.h"
#include "../context/inode_context.h"
#include "../data/inode.h"
#include "../error.h"

static const struct HsqsInode *
get_inode(const struct HsqsInodeDirectoryIndexIterator *iterator) {
	return (const struct HsqsInode *)hsqs_metablock_stream_data(
			&iterator->inode->metablock);
}

// TODO: use hsqs_data_inode_directory_ext_index().
static const struct HsqsInodeDirectoryIndex *
current_directory_index(
		const struct HsqsInodeDirectoryIndexIterator *iterator) {
	const uint8_t *tmp = (const uint8_t *)get_inode(iterator);
	return (const struct HsqsInodeDirectoryIndex
					*)&tmp[iterator->current_offset];
}

static int
directory_index_data_more(
		struct HsqsInodeDirectoryIndexIterator *iterator, size_t size) {
	return hsqs_metablock_stream_more(&iterator->inode->metablock, size);
}

int
hsqs_inode_directory_index_iterator_init(
		struct HsqsInodeDirectoryIndexIterator *iterator,
		struct HsqsInodeContext *inode) {
	int rv = 0;

	if (hsqs_inode_type(inode) != HSQS_INODE_TYPE_DIRECTORY ||
		hsqs_inode_is_extended(inode) == false) {
		return -HSQS_ERROR_NO_EXTENDED_DIRECTORY;
	}

	iterator->inode = inode;
	iterator->current_offset = 0;
	iterator->next_offset =
			HSQS_SIZEOF_INODE_HEADER + HSQS_SIZEOF_INODE_DIRECTORY_EXT;

	const struct HsqsInodeDirectoryExt *xdir =
			hsqs_data_inode_directory_ext(get_inode(iterator));
	iterator->remaining_entries =
			hsqs_data_inode_directory_ext_index_count(xdir);
	return rv;
}

int
hsqs_inode_directory_index_iterator_next(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	int rv = 0;
	iterator->current_offset = iterator->next_offset;

	iterator->remaining_entries--;

	// Make sure next entry is loaded:
	iterator->next_offset += HSQS_SIZEOF_INODE_DIRECTORY_INDEX;
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	// Make sure current index has its name populated
	iterator->next_offset +=
			hsqs_inode_directory_index_iterator_name_size(iterator);
	rv = directory_index_data_more(iterator, iterator->next_offset);
	if (rv < 0) {
		return rv;
	}

	return iterator->remaining_entries;
}

uint32_t
hsqs_inode_directory_index_iterator_index(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_index(current);
}
uint32_t
hsqs_inode_directory_index_iterator_start(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_start(current);
}
uint32_t
hsqs_inode_directory_index_iterator_name_size(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return hsqs_data_inode_directory_index_name_size(current) + 1;
}
const char *
hsqs_inode_directory_index_iterator_name(
		struct HsqsInodeDirectoryIndexIterator *iterator) {
	const struct HsqsInodeDirectoryIndex *current =
			current_directory_index(iterator);
	return (const char *)hsqs_data_inode_directory_index_name(current);
}

int
hsqs_inode_directory_index_iterator_clean(
		struct HsqsInodeDirectoryIndexIterator __attribute__((unused)) *
		iterator) {
	return 0;
}
