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
squash_fragment_init(struct SquashFragmentContext *fragment,
		const struct SquashSuperblockContext *superblock,
		const struct SquashInodeContext *inode) {
	int rv = 0;
	if (squash_data_superblock_flags(superblock->superblock) &
			SQUASH_SUPERBLOCK_NO_FRAGMENTS) {
		rv = -SQUASH_ERROR_NO_FRAGMENT;
		goto out;
	}
	fragment->inode = inode;
	fragment->superblock = superblock;

	uint32_t fragment_table_count =
			squash_data_superblock_fragment_entry_count(superblock->superblock);
	uint32_t fragment_table_start =
			squash_data_superblock_fragment_table_start(superblock->superblock);
	rv = squash_table_init(&fragment->table, superblock, fragment_table_start,
			SQUASH_SIZEOF_FRAGMENT, fragment_table_count);
	if (rv < 0) {
		goto out;
	}
	uint32_t index = squash_inode_file_fragment_block_index(inode);
	if (index == SQUASH_INODE_NO_FRAGMENT) {
		rv = -SQUASH_ERROR_NO_FRAGMENT;
		goto out;
	}
	rv = squash_table_get(
			&fragment->table, index, (const void **)&fragment->fragment);
	if (rv < 0) {
		goto out;
	}

	uint32_t block_size =
			squash_data_superblock_block_size(superblock->superblock);
	rv = squash_buffer_init(&fragment->buffer, superblock, block_size);
	if (rv < 0) {
		goto out;
	}

out:
	if (rv < 0) {
		squash_fragment_clean(fragment);
	}
	return rv;
}

int
squash_fragment_read(struct SquashFragmentContext *fragment) {
	const struct SquashDatablockSize *size_info =
			squash_data_fragment_size_info(fragment->fragment);
	bool is_compressed = squash_data_datablock_is_compressed(size_info);
	uint32_t size = squash_data_datablock_size(size_info);
	uint64_t start = squash_data_fragment_start(fragment->fragment);
	const uint8_t *tmp = (const uint8_t *)fragment->superblock;

	return squash_buffer_append(
			&fragment->buffer, &tmp[start], size, is_compressed);
}

uint32_t
squash_fragment_size(struct SquashFragmentContext *fragment) {
	uint64_t file_size = squash_inode_file_size(fragment->inode);
	uint32_t block_size =
			squash_data_superblock_block_size(fragment->superblock->superblock);

	return file_size % block_size;
}

const uint8_t *
squash_fragment_data(struct SquashFragmentContext *fragment) {
	const uint8_t *data = squash_buffer_data(&fragment->buffer);
	const uint32_t offset =
			squash_inode_file_fragment_block_offset(fragment->inode);

	return &data[offset];
}

int
squash_fragment_clean(struct SquashFragmentContext *fragment) {
	squash_table_cleanup(&fragment->table);
	squash_buffer_cleanup(&fragment->buffer);

	return 0;
}
