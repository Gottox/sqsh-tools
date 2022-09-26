/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         compression_option_context.c
 */

#include "compression_options_context.h"
#include "../data/compression_options.h"
#include "../data/superblock.h"
#include "../sqsh.h"
#include "metablock_context.h"
#include "superblock_context.h"

int
sqsh_compression_options_init(
		struct SqshCompressionOptionsContext *context, struct Sqsh *sqsh) {
	int rv = 0;
	struct SqshMetablockContext metablock = {0};

	rv = sqsh_metablock_init(&metablock, sqsh, HSQS_SIZEOF_SUPERBLOCK);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_buffer_init(&context->buffer);
	if (rv < 0) {
		goto out;
	}

	rv = sqsh_metablock_to_buffer(&metablock, &context->buffer);
	if (rv < 0) {
		goto out;
	}

out:
	sqsh_metablock_cleanup(&metablock);
	if (rv < 0) {
		sqsh_compression_options_cleanup(context);
	}
	return rv;
}

const union SqshCompressionOptions *
sqsh_compression_options_data(
		const struct SqshCompressionOptionsContext *context) {
	return (const union SqshCompressionOptions *)sqsh_buffer_data(
			&context->buffer);
}

size_t
sqsh_compression_options_size(
		const struct SqshCompressionOptionsContext *context) {
	return sqsh_buffer_size(&context->buffer);
}

int
sqsh_compression_options_cleanup(
		struct SqshCompressionOptionsContext *context) {
	sqsh_buffer_cleanup(&context->buffer);

	return 0;
}
