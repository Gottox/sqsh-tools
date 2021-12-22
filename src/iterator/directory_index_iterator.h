/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory_index_iterator
 * @created     : Monday Dec 20, 2021 12:39:21 CET
 */

#include "../utils.h"

#ifndef DIRECTORY_INDEX_ITERATOR_H

#define DIRECTORY_INDEX_ITERATOR_H

struct HsqsInodeContext;

struct HsqsInodeDirectoryIndexIterator {
	struct HsqsInodeContext *inode;
	const struct HsqsInodeDirectoryIndex *indices;
	size_t remaining_entries;
	hsqs_index_t current_offset;
	hsqs_index_t next_offset;
};

HSQS_NO_UNUSED int hsqs_inode_directory_index_iterator_init(
		struct HsqsInodeDirectoryIndexIterator *iterator,
		struct HsqsInodeContext *inode);
HSQS_NO_UNUSED int hsqs_inode_directory_index_iterator_next(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_index(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_start(
		struct HsqsInodeDirectoryIndexIterator *iterator);
uint32_t hsqs_inode_directory_index_iterator_name_size(
		struct HsqsInodeDirectoryIndexIterator *iterator);
const char *hsqs_inode_directory_index_iterator_name(
		struct HsqsInodeDirectoryIndexIterator *iterator);

HSQS_NO_UNUSED int hsqs_inode_directory_index_iterator_clean(
		struct HsqsInodeDirectoryIndexIterator *iterator);

#endif /* end of include guard DIRECTORY_INDEX_ITERATOR_H */
