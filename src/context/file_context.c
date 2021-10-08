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
 * @file        : file_context
 * @created     : Thursday Oct 07, 2021 09:27:20 CEST
 */

#include "file_context.h"
#include "../compression/buffer.h"
#include "../data/superblock.h"
#include "../error.h"
#include "datablock_context.h"
#include "fragment_context.h"
#include "inode_context.h"
#include <stdint.h>

int
squash_file_init(struct SquashFileContext *context,
		const struct SquashSuperblock *superblock,
		const struct SquashInodeContext *inode) {
	int rv = 0;
	rv = squash_datablock_init(&context->datablock, superblock, inode);
	if (rv < 0) {
		return rv;
	}
	rv = squash_fragment_init(&context->fragment, superblock, inode);
	if (rv < 0 && rv != -SQUASH_ERROR_NO_FRAGMENT) {
		return rv;
	}
	context->superblock = superblock;
	context->fragment_pos = UINT32_MAX;

	return squash_file_seek(context, 0);
}

int
squash_file_seek(struct SquashFileContext *context, uint64_t seek_pos) {
	int rv;
	rv = squash_datablock_seek(&context->datablock, seek_pos);
	if (rv == -SQUASH_ERROR_SEEK_IN_FRAGMENT) {
		context->fragment_pos = seek_pos %
				squash_data_superblock_block_size(context->superblock);
	} else {
		context->fragment_pos = UINT32_MAX;
	}

	return rv;
}

int
squash_file_read(struct SquashFileContext *context, uint64_t size) {
	int rv = 0;

	if (context->fragment_pos != UINT32_MAX) {
		return squash_fragment_read(&context->fragment);
	}

	rv = squash_datablock_read(&context->datablock, size);
	if (size > squash_file_size(context)) {
		rv = squash_fragment_read(&context->fragment);
		if (rv < 0) {
			goto out;
		}

		const uint8_t *fragment_data = squash_fragment_data(&context->fragment);
		uint64_t fragment_size = squash_fragment_size(&context->fragment);

		rv = squash_buffer_append(&context->datablock.buffer, fragment_data,
				fragment_size, false);
		if (rv < 0) {
			goto out;
		}
	}

out:
	return rv;
}

const uint8_t *
squash_file_data(struct SquashFileContext *context) {
	const uint8_t *data;
	if (context->fragment_pos != UINT32_MAX) {
		data = squash_fragment_data(&context->fragment);
		return &data[context->fragment_pos];
	} else {
		return squash_datablock_data(&context->datablock);
	}
}

uint64_t
squash_file_size(struct SquashFileContext *context) {
	uint64_t size;
	if (context->fragment_pos != UINT32_MAX) {
		size = squash_fragment_size(&context->fragment);
		if (context->fragment_pos > size) {
			return 0;
		} else {
			return size - context->fragment_pos;
		}
	} else {
		return squash_datablock_size(&context->datablock);
	}
}

int
squash_file_cleanup(struct SquashFileContext *context) {
	squash_datablock_clean(&context->datablock);
	squash_fragment_clean(&context->fragment);

	return 0;
}
