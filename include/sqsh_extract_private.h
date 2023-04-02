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
 * @file         sqsh_extract_private.h
 */

#ifndef SQSH_EXTRACT_PRIVATE_H
#define SQSH_EXTRACT_PRIVATE_H

#include "sqsh_primitive_private.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;
struct SqshMapReader;

////////////////////////////////////////
// extract/extract.c

typedef uint8_t sqsh__extractor_context_t[256];

struct SqshExtractorImpl {
	int (*init)(void *context, uint8_t *target, size_t target_size);
	int (*extract)(
			void *context, const uint8_t *compressed,
			const size_t compressed_size);
	int (*finish)(void *context, uint8_t *target, size_t *target_size);
};

struct SqshExtractor {
	/**
	 * @privatesection
	 */
	const struct SqshExtractorImpl *impl;
	size_t block_size;
};

/**
 * @internal
 * @memberof SqshExtractor
 * @brief Initializes a extractor context.
 *
 * @param[out] extractor      The context to initialize.
 * @param[in]  algorithm_id   The id of the compression algorithm to use.
 * @param[in]  block_size     The block size to use for the extraction.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__extractor_init(
		struct SqshExtractor *extractor, int algorithm_id,
		size_t block_size);

/**
 * @internal
 * @memberof SqshExtractor
 * @brief Decompresses data to a buffer.
 *
 * @param[in]     extractor     The extractor context to use.
 * @param[out]    buffer          The buffer to store the decompressed data.
 * @param[in]     compressed      The compressed data to decompress.
 * @param[in]     compressed_size The size of the compressed data.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__extractor_to_buffer(
		const struct SqshExtractor *extractor, struct SqshBuffer *buffer,
		const uint8_t *compressed, const size_t compressed_size);

/**
 * @internal
 * @memberof SqshExtractor
 * @brief returns the block size of the extractor
 *
 * @param[in]     extractor     The extractor context to use.
 *
 * @return the block size of the extractor context.
 */
size_t sqsh__extractor_block_size(const struct SqshExtractor *extractor);

/**
 * @internal
 * @memberof SqshExtractor
 * @brief Cleans up a extractor context.
 *
 * @param[in] extractor The context to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh__extractor_cleanup(struct SqshExtractor *extractor);

////////////////////////////////////////
// extract/extract_buffer.c

struct SqshExtractBuffer {
	struct SqshBuffer buffer;
	const uint8_t *compressed;
	size_t compressed_size;
};

int
sqsh__extract_buffer_init(void *context, uint8_t *target, size_t target_size);
int sqsh__extract_buffer_decompress(
		void *context, const uint8_t *compressed, const size_t compressed_size);
size_t sqsh__extract_buffer_size(void *context);
const uint8_t *sqsh__extract_buffer_data(void *context);
int sqsh__extract_buffer_cleanup(void *context);

////////////////////////////////////////
// extract/extract_manager.c

struct SqshExtractManager {
	struct SqshRcHashMap hash_map;
	const struct SqshExtractor *extractor;
	struct SqshMapManager *map_manager;
	struct SqshLru lru;
	pthread_mutex_t lock;
};

SQSH_NO_UNUSED int sqsh__extract_manager_init(
		struct SqshExtractManager *manager, struct SqshArchive *archive,
		const struct SqshExtractor *extractor, size_t size);

size_t sqsh__extract_manager_size(struct SqshExtractManager *manager);

int sqsh__extract_manager_uncompress(
		struct SqshExtractManager *manager, const struct SqshMapReader *reader,
		const struct SqshBuffer **target);

int sqsh__extract_manager_release(
		struct SqshExtractManager *manager, const struct SqshBuffer *buffer);

int sqsh__extract_manager_cleanup(struct SqshExtractManager *manager);

////////////////////////////////////////
// extract/extract_view.c

struct SqshExtractView {
	/**
	 * @privatesection
	 */
	struct SqshExtractManager *manager;
	const struct SqshBuffer *buffer;
	size_t offset;
	size_t size;
};

SQSH_NO_UNUSED int sqsh__extract_view_init(
		struct SqshExtractView *view, struct SqshExtractManager *manager,
		const struct SqshMapReader *reader);

SQSH_NO_UNUSED int sqsh__extract_view_narrow(
		struct SqshExtractView *view, sqsh_index_t offset, size_t size);

const uint8_t *sqsh__extract_view_data(const struct SqshExtractView *view);

size_t sqsh__extract_view_size(const struct SqshExtractView *view);

int sqsh__extract_view_cleanup(struct SqshExtractView *view);

////////////////////////////////////////
// extract/lz4.c

extern const struct SqshExtractorImpl *const sqsh__impl_lz4;
////////////////////////////////////////
// extract/lzma.c

extern const struct SqshExtractorImpl *const sqsh__impl_lzma;
extern const struct SqshExtractorImpl *const sqsh__impl_xz;

////////////////////////////////////////
// extract/lzo.c

extern const struct SqshExtractorImpl *const sqsh__impl_lzo;

////////////////////////////////////////
// extract/zlib.c

extern const struct SqshExtractorImpl *const sqsh__impl_zlib;

////////////////////////////////////////
// extract/zstd.c

extern const struct SqshExtractorImpl *const sqsh__impl_zstd;

#ifdef __cplusplus
}
#endif
#endif // SQSH_EXTRACT_PRIVATE_H
