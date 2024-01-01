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
 * @file         inode_builder.c
 */

#include "sqsh_error.h"
#define _DEFAULT_SOURCE

#include <mksqsh_metablock.h>
#include <sqsh_common_private.h>
#include <sqsh_data_set.h>
#include <string.h>

int
mksqsh__metablock_init(struct MksqshMetablock *metablock, FILE *out) {
	memset(metablock, 0, sizeof(*metablock));
	metablock->out = out;
	return 0;
}

int
mksqsh__metablock_write(
		struct MksqshMetablock *metablock, const uint8_t *data, size_t size) {
	metablock->flushed = false;
	int rv = 0;
	while (size > 0) {
		uint8_t *remaining_buffer = &metablock->buffer[metablock->buffer_size];
		const size_t remaining_size =
				sizeof(metablock->buffer) - metablock->buffer_size;
		const size_t copy_size = SQSH_MIN(remaining_size, size);

		memcpy(remaining_buffer, data, copy_size);
		metablock->buffer_size += copy_size;
		data += copy_size;
		size -= copy_size;
		if (metablock->buffer_size == sizeof(metablock->buffer)) {
			rv = mksqsh__metablock_flush(metablock);
			if (rv < 0) {
				goto out;
			}
		}
	}
out:
	return rv;
}

uint64_t
mksqsh__metablock_ref(const struct MksqshMetablock *metablock) {
	return sqsh_address_ref_create(
			metablock->outer_ref, metablock->buffer_size);
}

int
mksqsh__metablock_flush(struct MksqshMetablock *metablock) {
	int rv = 0;
	struct SqshDataMetablock header = {0};
	if (metablock->buffer_size == 0) {
		goto out;
	}

	sqsh__data_metablock_is_compressed_set(&header, false);
	sqsh__data_metablock_size_set(&header, metablock->buffer_size);

	rv = fwrite(&header, sizeof(header), 1, metablock->out);
	if (rv != 1) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	rv = fwrite(&metablock->buffer, metablock->buffer_size, 1, metablock->out);
	if (rv != 1) {
		rv = -SQSH_ERROR_INTERNAL;
		goto out;
	}

	metablock->outer_ref = sizeof(header) + metablock->buffer_size;
	metablock->buffer_size = 0;
	metablock->flushed = true;

	rv = 0;
out:
	return rv;
}

int
mksqsh__metablock_cleanup(struct MksqshMetablock *metablock) {
	(void)metablock;
	return 0;
}
