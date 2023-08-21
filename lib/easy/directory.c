/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         file.c
 */

#include <stdint.h>
#define _DEFAULT_SOURCE

#include "../../include/sqsh_easy.h"

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/sqsh_error.h"
#include "../../include/sqsh_file_private.h"
#include "../../include/sqsh_tree_private.h"

char **
sqsh_easy_directory_list(
		struct SqshArchive *archive, const char *path, int *err) {
	int rv = 0;
	static const uintptr_t nullptr = 0;
	struct SqshBuffer dir_list = {0};
	struct SqshBuffer dir_list_names = {0};
	size_t elements = 0;
	struct SqshFile *file = NULL;
	struct SqshDirectoryIterator *iterator = NULL;
	char **dir_list_data = NULL;

	file = sqsh_open(archive, path, &rv);
	if (rv < 0) {
		goto out;
	}

	if (sqsh_file_type(file) != SQSH_FILE_TYPE_DIRECTORY) {
		rv = -SQSH_ERROR_NOT_A_DIRECTORY;
		goto out;
	}

	rv = sqsh__buffer_init(&dir_list);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh__buffer_init(&dir_list_names);
	if (rv < 0) {
		goto out;
	}

	iterator = sqsh_directory_iterator_new(file, &rv);
	if (rv < 0) {
		goto out;
	}

	while (sqsh_directory_iterator_next(iterator, &rv)) {
		const char *name = sqsh_directory_iterator_name(iterator);
		size_t name_len = sqsh_directory_iterator_name_size(iterator);
		size_t index = sqsh__buffer_size(&dir_list_names);
		char *index_ptr = (void *)index;
		elements++;
		rv = sqsh__buffer_append(
				&dir_list, (uint8_t *)&index_ptr, sizeof(char *));
		if (rv < 0) {
			goto out;
		}
		rv = sqsh__buffer_append(&dir_list_names, (uint8_t *)name, name_len);
		if (rv < 0) {
			goto out;
		}
		rv = sqsh__buffer_append(
				&dir_list_names, (uint8_t *)&nullptr, sizeof(char));
		if (rv < 0) {
			goto out;
		}
	}

	rv = sqsh__buffer_append(&dir_list, (uint8_t *)&nullptr, sizeof(char *));
	size_t base_size = sqsh__buffer_size(&dir_list);

	const uint8_t *names_data = sqsh__buffer_data(&dir_list_names);
	size_t names_size = sqsh__buffer_size(&dir_list_names);
	rv = sqsh__buffer_append(&dir_list, names_data, names_size);
	if (rv < 0) {
		goto out;
	}

	dir_list_data = (char **)sqsh__buffer_unwrap(&dir_list);

	for (sqsh_index_t i = 0; i < elements; i++) {
		dir_list_data[i] += base_size + (uintptr_t)dir_list_data;
	}

out:
	sqsh_directory_iterator_free(iterator);
	sqsh__buffer_cleanup(&dir_list);
	sqsh__buffer_cleanup(&dir_list_names);
	sqsh_close(file);
	if (err) {
		*err = rv;
	}
	return dir_list_data;
}
