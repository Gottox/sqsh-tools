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
 * @file         directory_iterator.c
 */

#include <sqsh_directory_private.h>

#include <cextras/memory.h>
#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>
#include <sqsh_file_private.h>

static uint64_t
get_upper_limit(const struct SqshSuperblock *superblock) {
	if (sqsh_superblock_has_fragments(superblock)) {
		return sqsh_superblock_fragment_table_start(superblock);
	} else if (sqsh_superblock_has_export_table(superblock)) {
		return sqsh_superblock_export_table_start(superblock);
	} else {
		return sqsh_superblock_id_table_start(superblock);
	}
}

static int
load_metablock(
		struct SqshDirectoryIterator *iterator, const uint64_t outer_offset,
		uint16_t inner_offset) {
	const struct SqshFile *file = iterator->file;
	struct SqshArchive *archive = file->archive;
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);

	uint64_t start_address = sqsh_superblock_directory_table_start(superblock);
	const uint64_t upper_limit = get_upper_limit(superblock);
	if (SQSH_ADD_OVERFLOW(start_address, outer_offset, &start_address)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	iterator->next_offset = inner_offset;
	iterator->remaining_entries = 0;

	return sqsh__metablock_reader_init(
			&iterator->metablock, archive, start_address, upper_limit);
}

static const struct SqshDataDirectoryEntry *
get_entry(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *data = sqsh__metablock_reader_data(&iterator->metablock);
	return (const struct SqshDataDirectoryEntry *)data;
}

static const struct SqshDataDirectoryFragment *
get_fragment(const struct SqshDirectoryIterator *iterator) {
	const uint8_t *data = sqsh__metablock_reader_data(&iterator->metablock);
	return (const struct SqshDataDirectoryFragment *)data;
}

static int
directory_iterator_index_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;
	struct SqshDirectoryIndexIterator index_iterator = {0};
	const struct SqshFile *file = iterator->file;
	const uint64_t inode_ref = sqsh_file_inode_ref(file);
	uint64_t outer_offset = sqsh_file_directory_block_start(file);
	uint32_t dir_index = 0;

	rv = sqsh__directory_index_iterator_init(
			&index_iterator, file->archive, inode_ref);
	if (rv < 0) {
		goto out;
	}
	while (sqsh__directory_index_iterator_next(&index_iterator, &rv)) {
		const char *index_name =
				sqsh__directory_index_iterator_name(&index_iterator);
		const uint32_t index_name_size =
				sqsh__directory_index_iterator_name_size(&index_iterator);

		const size_t cmp_size = SQSH_MIN(index_name_size, name_len);
		/* BUG: the branch could be taken too early when the name is a prefix */
		if (strncmp(name, index_name, cmp_size) < 0) {
			break;
		}
		outer_offset = sqsh__directory_index_iterator_start(&index_iterator);
		dir_index = sqsh__directory_index_iterator_index(&index_iterator);
	}
	if (rv < 0) {
		goto out;
	}

	sqsh__metablock_reader_cleanup(&iterator->metablock);
	uint16_t inner_offset =
			(iterator->next_offset + dir_index) % SQSH_METABLOCK_BLOCK_SIZE;
	iterator->remaining_size -= dir_index;
	rv = load_metablock(iterator, outer_offset, inner_offset);
	iterator->remaining_entries = 0;
	if (rv < 0) {
		goto out;
	}
out:
	sqsh__directory_index_iterator_cleanup(&index_iterator);
	return rv;
}

static int
check_file_consistency(
		const struct SqshDirectoryIterator *iterator,
		const struct SqshFile *file) {
	const uint32_t file_inode = sqsh_file_inode(file);
	const uint32_t iter_inode = sqsh_directory_iterator_inode(iterator);
	if (iter_inode != file_inode) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	}

	enum SqshFileType file_type = sqsh_file_type(file);
	enum SqshFileType iter_type = sqsh_directory_iterator_file_type(iterator);
	if (iter_type != file_type) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	}
	return 0;
}

static int
process_fragment(struct SqshDirectoryIterator *iterator) {
	int rv = 0;

	rv = sqsh__metablock_reader_advance(
			&iterator->metablock, iterator->next_offset,
			sizeof(struct SqshDataDirectoryFragment));
	if (rv < 0) {
		return rv;
	}

	const struct SqshDataDirectoryFragment *fragment = get_fragment(iterator);
	/* entry count is 1-based. That means, that the actual amount of entries
	 * is one less than the value in the fragment header. We use this fact and
	 * don't decrease remaining_entries in the first iteration in a fragment.
	 * See _next() for details.
	 */
	iterator->remaining_entries = sqsh__data_directory_fragment_count(fragment);
	iterator->start_base = sqsh__data_directory_fragment_start(fragment);
	iterator->inode_base = sqsh__data_directory_fragment_inode_number(fragment);

	iterator->next_offset = sizeof(struct SqshDataDirectoryFragment);

	if (SQSH_SUB_OVERFLOW(
				iterator->remaining_size,
				sizeof(struct SqshDataDirectoryFragment),
				&iterator->remaining_size)) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_HEADER;
	}
	return rv;
}

static bool
directory_iterator_next(struct SqshDirectoryIterator *iterator, int *err) {
	int rv = 0;
	size_t size;
	bool has_next = false;

	if (iterator->remaining_size == 0) {
		if (iterator->remaining_entries != 0) {
			rv = -SQSH_ERROR_CORRUPTED_DIRECTORY_HEADER;
		}
		goto out;
	} else if (iterator->remaining_entries == 0) {
		/*  New fragment begins */
		rv = process_fragment(iterator);
		if (rv < 0) {
			goto out;
		}
	} else {
		iterator->remaining_entries--;
	}

	/*  Make sure next entry is loaded: */
	size = sizeof(struct SqshDataDirectoryEntry);
	rv = sqsh__metablock_reader_advance(
			&iterator->metablock, iterator->next_offset, size);
	if (rv < 0) {
		goto out;
	}

	size_t name_size;
	(void)sqsh_directory_iterator_name2(iterator, &name_size);
	/*  Make sure next entry has its name populated */
	if (SQSH_ADD_OVERFLOW(size, name_size, &size)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}
	rv = sqsh__metablock_reader_advance(&iterator->metablock, 0, size);
	if (rv < 0) {
		goto out;
	}

	iterator->next_offset = size;
	if (SQSH_SUB_OVERFLOW(
				iterator->remaining_size, size, &iterator->remaining_size)) {
		rv = -SQSH_ERROR_INTEGER_OVERFLOW;
		goto out;
	}

	has_next = true;
out:
	if (err != NULL) {
		*err = rv;
	}
	return has_next;
}

static int
check_entry_name_consistency(const struct SqshDirectoryIterator *iterator) {
	size_t name_len;
	const char *name = sqsh_directory_iterator_name2(iterator, &name_len);

	if (memchr(name, '\0', name_len) != NULL) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	} else if (memchr(name, '/', name_len) != NULL) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	}
	return 0;
}

static int
update_inode(struct SqshDirectoryIterator *iterator) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	const uint32_t inode_base = iterator->inode_base;
	const int16_t inode_offset = sqsh__data_directory_entry_inode_offset(entry);

	uint32_t inode;
	if (SQSH_ADD_OVERFLOW(inode_base, inode_offset, &inode)) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	}
	if (inode == 0) {
		return -SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY;
	}

	iterator->current_inode = (uint32_t)inode;
	return 0;
}

static int
directory_iterator_next_finalize(struct SqshDirectoryIterator *iterator) {
	int rv;

	rv = check_entry_name_consistency(iterator);
	if (rv < 0) {
		goto out;
	}

	rv = update_inode(iterator);
	if (rv < 0) {
		goto out;
	}
out:
	return rv;
}

int
sqsh_directory_iterator_lookup(
		struct SqshDirectoryIterator *iterator, const char *name,
		const size_t name_len) {
	int rv = 0;

	if (sqsh_file_is_extended(iterator->file)) {
		rv = directory_iterator_index_lookup(iterator, name, name_len);
		if (rv < 0) {
			return rv;
		}
	}

	while (directory_iterator_next(iterator, &rv) > 0) {
		size_t entry_name_size;
		const char *entry_name =
				sqsh_directory_iterator_name2(iterator, &entry_name_size);
		if (name_len != entry_name_size) {
			continue;
		}
		if (strncmp(name, entry_name, entry_name_size) == 0) {
			return directory_iterator_next_finalize(iterator);
		}
	}

	if (rv < 0) {
		return rv;
	} else {
		return -SQSH_ERROR_NO_SUCH_FILE;
	}
}

int
sqsh__directory_iterator_init(
		struct SqshDirectoryIterator *iterator, const struct SqshFile *file) {
	int rv = 0;

	if (sqsh_file_type(file) != SQSH_FILE_TYPE_DIRECTORY) {
		return -SQSH_ERROR_NOT_A_DIRECTORY;
	}

	iterator->file = file;

	if (SQSH_SUB_OVERFLOW(sqsh_file_size(file), 3, &iterator->remaining_size)) {
		return -SQSH_ERROR_INTEGER_OVERFLOW;
	}

	const uint64_t outer_offset = sqsh_file_directory_block_start(file);
	const uint16_t inner_offset = sqsh_file_directory_block_offset2(file);
	rv = load_metablock(iterator, outer_offset, inner_offset);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

struct SqshDirectoryIterator *
sqsh_directory_iterator_new(const struct SqshFile *file, int *err) {
	SQSH_NEW_IMPL(
			sqsh__directory_iterator_init, struct SqshDirectoryIterator, file);
}

uint16_t
sqsh_directory_iterator_name_size(
		const struct SqshDirectoryIterator *iterator) {
	size_t size;
	(void)sqsh_directory_iterator_name2(iterator, &size);
	return (uint16_t)size;
}

uint64_t
sqsh_directory_iterator_inode_ref(
		const struct SqshDirectoryIterator *iterator) {
	const uint32_t block_index = iterator->start_base;
	const uint16_t block_offset =
			sqsh__data_directory_entry_offset(get_entry(iterator));

	return sqsh_address_ref_create(block_index, block_offset);
}

uint32_t
sqsh_directory_iterator_inode(const struct SqshDirectoryIterator *iterator) {
	return iterator->current_inode;
}

uint64_t
sqsh_directory_iterator_inode_number(
		const struct SqshDirectoryIterator *iterator) {
	return sqsh_directory_iterator_inode(iterator);
}

enum SqshFileType
sqsh_directory_iterator_file_type(
		const struct SqshDirectoryIterator *iterator) {
	switch (sqsh__data_directory_entry_type(get_entry(iterator))) {
	case SQSH_INODE_TYPE_BASIC_DIRECTORY:
		return SQSH_FILE_TYPE_DIRECTORY;
	case SQSH_INODE_TYPE_BASIC_FILE:
		return SQSH_FILE_TYPE_FILE;
	case SQSH_INODE_TYPE_BASIC_SYMLINK:
		return SQSH_FILE_TYPE_SYMLINK;
	case SQSH_INODE_TYPE_BASIC_BLOCK:
		return SQSH_FILE_TYPE_BLOCK;
	case SQSH_INODE_TYPE_BASIC_CHAR:
		return SQSH_FILE_TYPE_CHAR;
	case SQSH_INODE_TYPE_BASIC_FIFO:
		return SQSH_FILE_TYPE_FIFO;
	case SQSH_INODE_TYPE_BASIC_SOCKET:
		return SQSH_FILE_TYPE_SOCKET;
	}
	return SQSH_FILE_TYPE_UNKNOWN;
}

struct SqshFile *
sqsh_directory_iterator_open_file(
		const struct SqshDirectoryIterator *iterator, int *err) {
	int rv = 0;
	struct SqshFile *file = NULL;
	const uint64_t inode_ref = sqsh_directory_iterator_inode_ref(iterator);
	const uint32_t dir_inode = sqsh_file_inode(iterator->file);
	struct SqshArchive *archive = iterator->file->archive;

	file = sqsh_open_by_ref(archive, inode_ref, &rv);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh__file_set_dir_inode(file, dir_inode);
	if (rv < 0) {
		goto out;
	}

	rv = check_file_consistency(iterator, file);

out:
	if (rv < 0) {
		sqsh_close(file);
		file = NULL;
	}
	if (err != NULL) {
		*err = rv;
	}
	return file;
}

bool
sqsh_directory_iterator_next(struct SqshDirectoryIterator *iterator, int *err) {
	int rv = 0;
	bool has_next = directory_iterator_next(iterator, &rv);
	if (has_next) {
		rv = directory_iterator_next_finalize(iterator);
	}

	if (err != NULL) {
		*err = rv;
	}
	if (rv < 0) {
		return false;
	}
	return has_next;
}

const char *
sqsh_directory_iterator_name2(
		const struct SqshDirectoryIterator *iterator, size_t *len) {
	const struct SqshDataDirectoryEntry *entry = get_entry(iterator);
	*len = sqsh__data_directory_entry_name_size(entry) + 1;
	return (const char *)sqsh__data_directory_entry_name(entry);
}

const char *
sqsh_directory_iterator_name(const struct SqshDirectoryIterator *iterator) {
	size_t dummy;
	return sqsh_directory_iterator_name2(iterator, &dummy);
}

char *
sqsh_directory_iterator_name_dup(const struct SqshDirectoryIterator *iterator) {
	size_t size;
	const char *entry_name = sqsh_directory_iterator_name2(iterator, &size);

	return cx_memdup(entry_name, size);
}

int
sqsh__directory_iterator_cleanup(struct SqshDirectoryIterator *iterator) {
	return sqsh__metablock_reader_cleanup(&iterator->metablock);
}

int
sqsh_directory_iterator_free(struct SqshDirectoryIterator *iterator) {
	SQSH_FREE_IMPL(sqsh__directory_iterator_cleanup, iterator);
}
