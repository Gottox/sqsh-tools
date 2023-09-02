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
 * @file         extractor2.c
 */

#include "../../include/sqsh_extract_private.h"

#include "../../include/sqsh_archive.h"
#include "../../include/sqsh_error.h"
#include <cextras/collection.h>

const struct SqshExtractorImpl *const __attribute__((weak)) sqsh__impl_lzo =
		NULL;

const struct SqshExtractorImpl *
sqsh__extractor2_impl_from_id(int id) {
	switch ((enum SqshSuperblockCompressionId)id) {
	case SQSH_COMPRESSION_GZIP:
		return sqsh__impl_zlib;
	case SQSH_COMPRESSION_LZMA:
		return sqsh__impl_lzma;
	case SQSH_COMPRESSION_XZ:
		return sqsh__impl_xz;
	case SQSH_COMPRESSION_LZO:
		return sqsh__impl_lzo;
	case SQSH_COMPRESSION_LZ4:
		return sqsh__impl_lz4;
	case SQSH_COMPRESSION_ZSTD:
		return sqsh__impl_zstd;
	default:
		return NULL;
	}
}

int
sqsh__extractor2_init(
		struct SqshExtractor2 *extractor, struct CxBuffer *buffer,
		int algorithm_id, size_t block_size) {
	int rv = 0;
	const struct SqshExtractorImpl *impl =
			sqsh__extractor2_impl_from_id(algorithm_id);
	if (impl == NULL) {
		rv = -SQSH_ERROR_COMPRESSION_UNSUPPORTED;
		goto out;
	}
	extractor->impl = impl;
	extractor->block_size = block_size;
	extractor->buffer = buffer;
	rv = cx_buffer_add_capacity(buffer, &extractor->target, block_size);
	if (rv < 0) {
		goto out;
	}

	rv = impl->init(&extractor->context, extractor->target, block_size);
	if (rv < 0) {
		goto out;
	}

out:
	return 0;
}

int
sqsh__extractor2_write(
		struct SqshExtractor2 *extractor, const uint8_t *compressed,
		const size_t compressed_size) {
	int rv = 0;
	const struct SqshExtractorImpl *impl = extractor->impl;
	sqsh__extractor_context_t *context = &extractor->context;
	if (impl == NULL) {
		rv = -SQSH_ERROR_COMPRESSION_FINISHED;
		goto out;
	}

	rv = impl->write(context, compressed, compressed_size);
out:
	return rv;
}

int
sqsh__extractor2_finish(struct SqshExtractor2 *extractor) {
	const struct SqshExtractorImpl *impl = extractor->impl;
	sqsh__extractor_context_t *context = &extractor->context;
	int rv = 0;
	size_t size = extractor->block_size;
	if (impl == NULL) {
		rv = -SQSH_ERROR_COMPRESSION_FINISHED;
		goto out;
	}

	rv = impl->finish(context, extractor->target, &size);
	if (rv < 0) {
		goto out;
	}

	rv = cx_buffer_add_size(extractor->buffer, size);
	if (rv < 0) {
		goto out;
	}
	extractor->impl = NULL;
out:
	return rv;
}

int
sqsh__extractor2_cleanup(struct SqshExtractor2 *extractor) {
	const struct SqshExtractorImpl *impl = extractor->impl;
	sqsh__extractor_context_t *context = &extractor->context;
	int rv = 0;
	// Make sure we cleanup the compressor in an error case.
	if (extractor->impl != NULL) {
		size_t dummy_size = 0;
		rv = impl->finish(context, extractor->target, &dummy_size);
	}
	extractor->impl = NULL;
	extractor->block_size = 0;
	extractor->buffer = NULL;
	return rv;
}
