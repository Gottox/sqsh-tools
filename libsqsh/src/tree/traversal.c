/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         traversal.c
 */

#include <sqsh_tree_private.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <stdbool.h>

#define RECURSION_CHECK_DEPTH 128

int
sqsh__tree_traversal_init(
		struct SqshTreeTraversal *traversal, const struct SqshFile *file) {
	int rv = 0;
	cx_pin_vec_init(
			&traversal->stack, sizeof(struct SqshTreeTraversalStackElement));
	rv = cx_pin_vec_reserve(&traversal->stack, 8);
	if (rv < 0) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}

	traversal->base_file = file;
	traversal->state = SQSH_TREE_TRAVERSAL_STATE_INIT;
	traversal->max_depth = SIZE_MAX;
	traversal->depth = 0;
	traversal->current_iterator = NULL;

	struct SqshTreeTraversalStackElement *base =
			cx_pin_vec_push(&traversal->stack, NULL);
	if (base == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = sqsh__file_init(&base->file, file->archive, sqsh_file_inode_ref(file));
	if (rv < 0) {
		goto out;
	}
	sqsh__file_set_parent_inode_ref(&base->file, file->parent_inode_ref);
	traversal->current_file = &base->file;

out:
	if (rv < 0) {
		cx_pin_vec_cleanup(&traversal->stack);
	}
	return rv;
}

struct SqshTreeTraversal *
sqsh_tree_traversal_new(const struct SqshFile *file, int *err) {
	SQSH_NEW_IMPL(sqsh__tree_traversal_init, struct SqshTreeTraversal, file);
}

void
sqsh_tree_traversal_set_max_depth(
		struct SqshTreeTraversal *traversal, size_t max_depth) {
	traversal->max_depth = max_depth;
}

static int
push_stack(struct SqshTreeTraversal *traversal) {
	struct SqshArchive *archive = traversal->base_file->archive;
	const uint64_t parent_inode_ref =
			sqsh_file_inode_ref(traversal->current_file);
	const uint64_t inode_ref =
			sqsh_directory_iterator_inode_ref(traversal->current_iterator);

	struct SqshTreeTraversalStackElement *element =
			cx_pin_vec_push(&traversal->stack, NULL);
	if (element == NULL) {
		return -SQSH_ERROR_MALLOC_FAILED;
	}
	int rv = sqsh__file_init(&element->file, archive, inode_ref);
	if (rv < 0) {
		cx_pin_vec_pull(&traversal->stack);
		return rv;
	}
	sqsh__file_set_parent_inode_ref(&element->file, parent_inode_ref);
	traversal->current_file = &element->file;
	traversal->state = SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN;
	return 0;
}

static int
pop_stack(struct SqshTreeTraversal *traversal) {
	traversal->state = SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END;

	struct SqshTreeTraversalStackElement *element =
			cx_pin_vec_peek(&traversal->stack);
	sqsh__file_cleanup(&element->file);
	sqsh__directory_iterator_cleanup(&element->iterator);
	cx_pin_vec_pull(&traversal->stack);

	struct SqshTreeTraversalStackElement *new_top =
			cx_pin_vec_peek(&traversal->stack);
	if (new_top == NULL) {
		traversal->current_iterator = NULL;
		traversal->current_file = traversal->base_file;
	} else {
		traversal->current_iterator = &new_top->iterator;
		traversal->current_file = &new_top->file;
	}
	return 0;
}

static bool
file_next(struct SqshTreeTraversal *traversal, int *err) {
	int rv = 0;
	if (traversal->current_iterator == NULL) {
		return false;
	} else {
		const bool has_next =
				sqsh_directory_iterator_next(traversal->current_iterator, err);
		if (!has_next) {
			pop_stack(traversal);
			traversal->depth--;
		} else if (
				traversal->depth < traversal->max_depth &&
				sqsh_directory_iterator_file_type(
						traversal->current_iterator) ==
						SQSH_FILE_TYPE_DIRECTORY) {
			rv = push_stack(traversal);
			if (rv < 0) {
				*err = rv;
				return false;
			}
		} else {
			traversal->state = SQSH_TREE_TRAVERSAL_STATE_FILE;
		}
		return true;
	}
}

static int
check_recursion(struct SqshTreeTraversal *traversal) {
	const size_t size = cx_pin_vec_size(&traversal->stack);
	if (size < 2) {
		return 0;
	}
	const struct SqshTreeTraversalStackElement *top =
			cx_pin_vec_get(&traversal->stack, size - 1);
	const uint64_t inode_ref = sqsh_file_inode_ref(&top->file);

	for (size_t i = 0; i + 1 < size; i++) {
		const struct SqshTreeTraversalStackElement *element =
				cx_pin_vec_get(&traversal->stack, i);
		if (sqsh_file_inode_ref(&element->file) == inode_ref) {
			return -SQSH_ERROR_DIRECTORY_RECURSION;
		}
	}
	return 0;
}

static bool
directory_begin_next(struct SqshTreeTraversal *traversal, int *err) {
	bool has_next = false;
	int rv = 0;
	traversal->depth++;
	if (traversal->depth >= RECURSION_CHECK_DEPTH) {
		rv = check_recursion(traversal);
		if (rv < 0) {
			goto out;
		}
	}

	struct SqshTreeTraversalStackElement *top =
			cx_pin_vec_peek(&traversal->stack);
	traversal->current_iterator = &top->iterator;
	rv = sqsh__directory_iterator_init(
			traversal->current_iterator, traversal->current_file);
	if (rv < 0) {
		goto out;
	}

	has_next = file_next(traversal, &rv);
	if (rv < 0) {
		goto out;
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	return has_next;
}

static bool
init_next(struct SqshTreeTraversal *traversal) {
	if (traversal->max_depth != 0 &&
		sqsh_file_type(traversal->base_file) == SQSH_FILE_TYPE_DIRECTORY) {
		traversal->state = SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN;
	} else {
		traversal->state = SQSH_TREE_TRAVERSAL_STATE_FILE;
	}
	return true;
}

bool
sqsh_tree_traversal_next(struct SqshTreeTraversal *traversal, int *err) {
	int dummy;
	if (err == NULL) {
		err = &dummy;
	}

	switch (traversal->state) {
	case SQSH_TREE_TRAVERSAL_STATE_INIT:
		return init_next(traversal);
	case SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_BEGIN:
		return directory_begin_next(traversal, err);
	case SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END:
	case SQSH_TREE_TRAVERSAL_STATE_FILE:
		return file_next(traversal, err);
	}
	return false;
}

enum SqshFileType
sqsh_tree_traversal_type(const struct SqshTreeTraversal *traversal) {
	if (traversal->current_iterator == NULL) {
		return sqsh_file_type(traversal->base_file);
	} else {
		return sqsh_directory_iterator_file_type(traversal->current_iterator);
	}
}

enum SqshTreeTraversalState
sqsh_tree_traversal_state(const struct SqshTreeTraversal *traversal) {
	return traversal->state;
}

const char *
sqsh_tree_traversal_name(
		const struct SqshTreeTraversal *traversal, size_t *len) {
	if (traversal->current_iterator == NULL) {
		*len = 0;
		return "";
	} else {
		return sqsh_directory_iterator_name2(traversal->current_iterator, len);
	}
}

const char *
sqsh_tree_traversal_path_segment(
		const struct SqshTreeTraversal *traversal, size_t *len, size_t index) {
	if (traversal->current_iterator == NULL) {
		*len = 0;
		return "";
	}

	if (index >= traversal->depth) {
		*len = 0;
		return NULL;
	}

	const struct SqshTreeTraversalStackElement *element =
			cx_pin_vec_get(&traversal->stack, index);
	return sqsh_directory_iterator_name2(&element->iterator, len);
}

size_t
sqsh_tree_traversal_depth(const struct SqshTreeTraversal *traversal) {
	return traversal->depth;
}

static size_t
total_path_length(const struct SqshTreeTraversal *traversal) {
	size_t length = 0;

	for (size_t i = 0; i < traversal->depth; i++) {
		const struct SqshTreeTraversalStackElement *element =
				cx_pin_vec_get(&traversal->stack, i);
		size_t segment_len = 0;
		sqsh_directory_iterator_name2(&element->iterator, &segment_len);
		length += segment_len + 1;
	}

	// The loop counted a separator after every segment; the actual path has
	// (depth - 1) separators (none after the last), so drop one.
	return length ? length - 1 : 0;
}

char *
sqsh_tree_traversal_path_dup(const struct SqshTreeTraversal *traversal) {
	size_t length = total_path_length(traversal);
	char *buffer = malloc(length + 1);
	if (buffer == NULL) {
		return NULL;
	}

	char *cursor = buffer;
	for (size_t i = 0; i < traversal->depth; i++) {
		const struct SqshTreeTraversalStackElement *element =
				cx_pin_vec_get(&traversal->stack, i);
		size_t segment_len = 0;
		const char *segment =
				sqsh_directory_iterator_name2(&element->iterator, &segment_len);
		memcpy(cursor, segment, segment_len);
		cursor += segment_len;
		*cursor++ = '/';
	}
	if (cursor > buffer) {
		cursor--; // drop the trailing '/'
	}
	*cursor = '\0';

	return buffer;
}

char *
sqsh_tree_traversal_name_dup(const struct SqshTreeTraversal *traversal) {
	return sqsh_directory_iterator_name_dup(traversal->current_iterator);
}

struct SqshFile *
sqsh_tree_traversal_open_file(
		const struct SqshTreeTraversal *traversal, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	if (traversal->current_iterator == NULL) {
		uint64_t inode_ref = sqsh_file_inode_ref(traversal->base_file);
		file = sqsh_open_by_ref(traversal->base_file->archive, inode_ref, &rv);
		if (rv < 0) {
			goto out;
		}
		sqsh__file_set_parent_inode_ref(
				file, traversal->base_file->parent_inode_ref);
	} else {
		file = sqsh_directory_iterator_open_file(
				traversal->current_iterator, &rv);
	}
out:
	if (err != NULL) {
		*err = rv;
	}
	if (rv < 0) {
		sqsh_close(file);
		return NULL;
	}
	return file;
}

int
sqsh__tree_traversal_cleanup(struct SqshTreeTraversal *traversal) {
	while (cx_pin_vec_size(&traversal->stack) > 0) {
		pop_stack(traversal);
	}
	cx_pin_vec_cleanup(&traversal->stack);
	return 0;
}

const struct SqshDirectoryIterator *
sqsh_tree_traversal_iterator(const struct SqshTreeTraversal *traversal) {
	return traversal->current_iterator;
}

int
sqsh_tree_traversal_free(struct SqshTreeTraversal *traversal) {
	SQSH_FREE_IMPL(sqsh__tree_traversal_cleanup, traversal);
}
