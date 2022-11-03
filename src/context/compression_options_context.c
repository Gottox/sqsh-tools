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

#include <sqsh.h>
#include <sqsh_context.h>
#include <sqsh_data.h>
#include <stdint.h>

static const union SqshCompressionOptions *
compression_options(const struct SqshCompressionOptionsContext *context) {
	return (union SqshCompressionOptions *)sqsh_buffer_data(&context->buffer);
}

int
sqsh_compression_options_init(
		struct SqshCompressionOptionsContext *context, struct Sqsh *sqsh) {
	int rv = 0;
	struct SqshMetablockContext metablock = {0};
	struct SqshSuperblockContext *superblock = sqsh_superblock(sqsh);

	rv = sqsh_metablock_init(&metablock, sqsh, SQSH_SIZEOF_SUPERBLOCK);
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
	context->compression_id = sqsh_superblock_compression_id(superblock);

out:
	sqsh_metablock_cleanup(&metablock);
	if (rv < 0) {
		sqsh_compression_options_cleanup(context);
	}
	return rv;
}

uint32_t
sqsh_compression_options_gzip_compression_level(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_GZIP) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_gzip_compression_level(
			compression_options(context));
}
uint16_t
sqsh_compression_options_gzip_window_size(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_GZIP) {
		return UINT16_MAX;
	}
	return sqsh_data_compression_options_gzip_window_size(
			compression_options(context));
}
enum SqshGzipStrategies
sqsh_compression_options_gzip_strategies(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_GZIP) {
		return UINT16_MAX;
	}
	return sqsh_data_compression_options_gzip_strategies(
			compression_options(context));
}

uint32_t
sqsh_compression_options_xz_dictionary_size(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_XZ) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_xz_dictionary_size(
			compression_options(context));
}
uint32_t
sqsh_compression_options_xz_filters(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_XZ) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_xz_filters(
			compression_options(context));
}

uint32_t
sqsh_compression_options_lz4_version(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_LZ4) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_lz4_version(
			compression_options(context));
}
uint32_t
sqsh_compression_options_lz4_flags(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_LZ4) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_lz4_flags(
			compression_options(context));
}

uint32_t
sqsh_compression_options_zstd_compression_level(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_ZSTD) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_zstd_compression_level(
			compression_options(context));
}

enum SqshLzoAlgorithm
sqsh_compression_options_lzo_algorithm(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_LZO) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_lzo_algorithm(
			compression_options(context));
}
uint32_t
sqsh_compression_options_lzo_compression_level(
		const struct SqshCompressionOptionsContext *context) {
	if (context->compression_id != SQSH_COMPRESSION_LZO) {
		return UINT32_MAX;
	}
	return sqsh_data_compression_options_lzo_compression_level(
			compression_options(context));
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
