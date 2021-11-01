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
 * @file        : fragment_context
 * @created     : Friday Sep 17, 2021 09:44:12 CEST
 */

#include "fragment_context.h"
#include "../data/fragment.h"
#include "../data/superblock.h"
#include "../error.h"
#include "inode_context.h"
#include "superblock_context.h"
#include <stdint.h>

int
hsqs_fragment_init(
		struct HsqsFragmentContext *fragment,
		const struct HsqsInodeContext *inode) {
	int rv = 0;
	fragment->inode = inode;
	fragment->superblock = inode->extract.superblock;
	uint32_t fragment_table_count =
			hsqs_superblock_fragment_entry_count(fragment->superblock);
	uint32_t fragment_table_start =
			hsqs_superblock_fragment_table_start(fragment->superblock);

	if (!hsqs_superblock_has_fragments(fragment->superblock)) {
		rv = -HSQS_ERROR_NO_FRAGMENT;
		goto out;
	}

	rv = hsqs_table_init(
			&fragment->table, fragment->superblock, fragment_table_start,
			HSQS_SIZEOF_FRAGMENT, fragment_table_count);
	if (rv < 0) {
		goto out;
	}
	uint32_t index = hsqs_inode_file_fragment_block_index(inode);
	if (index == HSQS_INODE_NO_FRAGMENT) {
		rv = -HSQS_ERROR_NO_FRAGMENT;
		goto out;
	}
	rv = hsqs_table_get(
			&fragment->table, index, (const void **)&fragment->fragment);
	if (rv < 0) {
		goto out;
	}

	uint32_t block_size = hsqs_superblock_block_size(fragment->superblock);
	rv = hsqs_buffer_init(&fragment->buffer, fragment->superblock, block_size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		hsqs_fragment_clean(fragment);
	}
	return rv;
}

int
hsqs_fragment_read(struct HsqsFragmentContext *fragment) {
	const struct HsqsDatablockSize *size_info =
			hsqs_data_fragment_size_info(fragment->fragment);
	bool is_compressed = hsqs_data_datablock_is_compressed(size_info);
	uint32_t size = hsqs_data_datablock_size(size_info);
	uint64_t start = hsqs_data_fragment_start(fragment->fragment);
	const uint8_t *data =
			hsqs_superblock_data_from_offset(fragment->superblock, start);

	return hsqs_buffer_append(&fragment->buffer, data, size, is_compressed);
}

uint32_t
hsqs_fragment_size(struct HsqsFragmentContext *fragment) {
	uint64_t file_size = hsqs_inode_file_size(fragment->inode);
	uint32_t block_size = hsqs_superblock_block_size(fragment->superblock);

	return file_size % block_size;
}

const uint8_t *
hsqs_fragment_data(struct HsqsFragmentContext *fragment) {
	const uint8_t *data = hsqs_buffer_data(&fragment->buffer);
	const uint32_t offset =
			hsqs_inode_file_fragment_block_offset(fragment->inode);

	return &data[offset];
}

int
hsqs_fragment_clean(struct HsqsFragmentContext *fragment) {
	hsqs_table_cleanup(&fragment->table);
	hsqs_buffer_cleanup(&fragment->buffer);

	return 0;
}
