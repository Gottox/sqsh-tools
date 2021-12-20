/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Friday May 07, 2021 06:56:03 CEST
 */

#include "../context/inode_context.h"
#include "../context/metablock_stream_context.h"
#include "../utils.h"
#include <stdint.h>

#ifndef HSQS_DIRECTORY_CONTEXT_H

#define HSQS_DIRECTORY_CONTEXT_H

struct HsqsInodeContext;
struct Hsqs;

struct HsqsDirectoryIterator {
	struct HsqsInodeContext *inode;
	uint32_t block_start;
	uint32_t block_offset;
	uint32_t size;

	const struct HsqsDirectoryFragment *fragments;
	struct HsqsDirectoryContext *directory;
	struct HsqsMetablockStreamContext metablock;
	size_t remaining_entries;
	hsqs_index_t current_fragment_offset;
	hsqs_index_t next_offset;
	hsqs_index_t current_offset;
};

HSQS_NO_UNUSED int hsqs_directory_iterator_init(
		struct HsqsDirectoryIterator *iterator, struct HsqsInodeContext *inode);
HSQS_NO_UNUSED int
hsqs_directory_iterator_next(struct HsqsDirectoryIterator *iterator);
HSQS_NO_UNUSED int hsqs_directory_iterator_lookup(
		struct HsqsDirectoryIterator *iterator, const char *name,
		const size_t name_len);
int
hsqs_directory_iterator_name_size(const struct HsqsDirectoryIterator *iterator);
uint64_t
hsqs_directory_iterator_inode_ref(const struct HsqsDirectoryIterator *iterator);
enum HsqsInodeContextType hsqs_directory_iterator_inode_type(
		const struct HsqsDirectoryIterator *iterator);
HSQS_NO_UNUSED int hsqs_directory_iterator_inode_load(
		const struct HsqsDirectoryIterator *iterator,
		struct HsqsInodeContext *inode);
const char *
hsqs_directory_iterator_name(const struct HsqsDirectoryIterator *iterator);
HSQS_NO_UNUSED int hsqs_directory_iterator_name_dup(
		const struct HsqsDirectoryIterator *iterator, char **name_buffer);
int hsqs_directory_iterator_cleanup(struct HsqsDirectoryIterator *iterator);
#endif /* end of include guard HSQS_DIRECTORY_CONTEXT_H */
