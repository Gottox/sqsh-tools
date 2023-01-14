/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : sqsh_iterator_private.h
 */

#ifndef SQSH_ITERATOR_PRIVATE_H
#define SQSH_ITERATOR_PRIVATE_H

#include "sqsh_context_private.h"
#include "sqsh_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

// iterator/directory_index_iterator.c

struct SqshInodeDirectoryIndexIterator {
	struct SqshInodeContext *inode;
	size_t remaining_entries;
	sqsh_index_t current_offset;
	sqsh_index_t next_offset;
};

SQSH_NO_UNUSED int sqsh__directory_index_iterator_init(
		struct SqshInodeDirectoryIndexIterator *iterator,
		struct SqshInodeContext *inode);
SQSH_NO_UNUSED
struct SqshInodeDirectoryIndexIterator *
sqsh__directory_index_iterator_new(struct SqshInodeContext *inode, int *err);
SQSH_NO_UNUSED int sqsh__directory_index_iterator_next(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh__directory_index_iterator_index(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh__directory_index_iterator_start(
		struct SqshInodeDirectoryIndexIterator *iterator);
uint32_t sqsh__directory_index_iterator_name_size(
		struct SqshInodeDirectoryIndexIterator *iterator);
const char *sqsh__directory_index_iterator_name(
		struct SqshInodeDirectoryIndexIterator *iterator);
int sqsh__directory_index_iterator_cleanup(
		struct SqshInodeDirectoryIndexIterator *iterator);
int sqsh__directory_index_iterator_free(
		struct SqshInodeDirectoryIndexIterator *iterator);

// iterator/directory_iterator.c

struct SqshDirectoryIterator {
	struct SqshInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;

	const struct SqshDirectoryFragment *fragments;
	struct SqshDirectoryContext *directory;
	struct SqshMetablockStreamContext metablock;
	size_t remaining_entries;
	sqsh_index_t current_fragment_offset;
	sqsh_index_t next_offset;
	sqsh_index_t current_offset;
};

SQSH_NO_UNUSED int sqsh__directory_iterator_init(
		struct SqshDirectoryIterator *iterator, struct SqshInodeContext *inode);
int sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator);

// iterator/xattr_iterator.c

struct SqshXattrIterator {
	struct SqshMetablockStreamContext metablock;
	struct SqshMetablockStreamContext out_of_line_value;
	struct SqshXattrTable *context;
	int remaining_entries;
	sqsh_index_t next_offset;
	sqsh_index_t key_offset;
	sqsh_index_t value_offset;
};

SQSH_NO_UNUSED int sqsh__xattr_iterator_init(
		struct SqshXattrIterator *iterator,
		const struct SqshInodeContext *inode);
int sqsh__xattr_iterator_cleanup(struct SqshXattrIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_ITERATOR_PRIVATE_H */
