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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression_option_context
 * @created     : Tuesday Nov 30, 2021 15:30:08 CET
 */

#include "compression_options_context.h"
#include "../data/compression_options.h"
#include "../data/superblock.h"
#include "../hsqs.h"
#include "metablock_context.h"
#include "superblock_context.h"

int
hsqs_compression_options_init(
		struct HsqsCompressionOptionsContext *context, struct Hsqs *hsqs) {
	int rv = 0;
	struct HsqsMetablockContext metablock = {0};

	rv = hsqs_metablock_init(&metablock, hsqs, HSQS_SIZEOF_SUPERBLOCK);
	if (rv < 0) {
		goto out;
	}

	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	enum HsqsSuperblockCompressionId compression_id =
			hsqs_superblock_compression_id(superblock);
	rv = hsqs_buffer_init(
			&context->buffer, compression_id, HSQS_METABLOCK_BLOCK_SIZE);
	if (rv < 0) {
		goto out;
	}

	rv = hsqs_metablock_to_buffer(&metablock, &context->buffer);
	if (rv < 0) {
		goto out;
	}

out:
	hsqs_metablock_cleanup(&metablock);
	if (rv < 0) {
		hsqs_compression_options_cleanup(context);
	}
	return rv;
}

const union HsqsCompressionOptions *
hsqs_compression_options_data(
		const struct HsqsCompressionOptionsContext *context) {
	return (const union HsqsCompressionOptions *)hsqs_buffer_data(
			&context->buffer);
}

size_t
hsqs_compression_options_size(
		const struct HsqsCompressionOptionsContext *context) {
	return hsqs_buffer_size(&context->buffer);
}

int
hsqs_compression_options_cleanup(
		struct HsqsCompressionOptionsContext *context) {
	hsqs_buffer_cleanup(&context->buffer);

	return 0;
}
