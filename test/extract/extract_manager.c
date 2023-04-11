/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2023, Enno Boland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         extract_manager.c
 */

#include "../common.h"
#include "../test.h"

#include "../../include/sqsh_archive_private.h"
#include "../../include/sqsh_data_private.h"
#include "../../include/sqsh_extract_private.h"
#include "../../include/sqsh_mapper_private.h"

static void
decompress(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshExtractManager manager = {0};
	const struct SqshBuffer *buffer = NULL;
	uint8_t payload[] = {SQSH_HEADER, ZLIB_ABCD};

	mk_stub(&archive, payload, sizeof(payload));

	struct SqshMapManager *map_manager = sqsh_archive_map_manager(&archive);
	const struct SqshExtractor *compression =
			sqsh_archive_metablock_extractor(&archive);
	struct SqshMapReader reader = {0};
	rv = sqsh__map_reader_init(
			&reader, map_manager, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__map_reader_advance(&reader, 0, CHUNK_SIZE(ZLIB_ABCD));
	assert(rv == 0);

	rv = sqsh__extract_manager_init(&manager, &archive, compression, 10);
	assert(rv == 0);

	rv = sqsh__extract_manager_uncompress(&manager, &reader, &buffer);
	assert(rv == 0);
	assert(buffer != NULL);
	assert(sqsh__buffer_size(buffer) == 4);
	assert(memcmp(sqsh__buffer_data(buffer), "abcd", 4) == 0);

	sqsh__map_reader_cleanup(&reader);
	sqsh__extract_manager_release(&manager, buffer);
	sqsh__extract_manager_cleanup(&manager);
	sqsh__archive_cleanup(&archive);
}

static void
decompress_and_cached(void) {
	int rv;
	struct SqshArchive archive = {0};
	struct SqshExtractManager manager = {0};
	const struct SqshBuffer *buffer = NULL;
	const struct SqshBuffer *cached_buffer = NULL;
	uint8_t payload[] = {SQSH_HEADER, ZLIB_ABCD};

	mk_stub(&archive, payload, sizeof(payload));

	struct SqshMapManager *map_manager = sqsh_archive_map_manager(&archive);
	const struct SqshExtractor *compression =
			sqsh_archive_metablock_extractor(&archive);
	struct SqshMapReader reader = {0};
	rv = sqsh__map_reader_init(
			&reader, map_manager, SQSH_SIZEOF_SUPERBLOCK, sizeof(payload));
	assert(rv == 0);

	rv = sqsh__map_reader_advance(&reader, 0, CHUNK_SIZE(ZLIB_ABCD));
	assert(rv == 0);

	rv = sqsh__extract_manager_init(&manager, &archive, compression, 10);
	assert(rv == 0);

	rv = sqsh__extract_manager_uncompress(&manager, &reader, &buffer);
	assert(rv == 0);
	assert(buffer != NULL);
	assert(sqsh__buffer_size(buffer) == 4);
	assert(memcmp(sqsh__buffer_data(buffer), "abcd", 4) == 0);

	rv = sqsh__extract_manager_uncompress(&manager, &reader, &cached_buffer);
	assert(rv == 0);
	assert(buffer == cached_buffer);

	sqsh__map_reader_cleanup(&reader);
	sqsh__extract_manager_release(&manager, buffer);
	sqsh__extract_manager_release(&manager, cached_buffer);
	sqsh__extract_manager_cleanup(&manager);
	sqsh__archive_cleanup(&archive);
}

DEFINE
TEST(decompress);
TEST(decompress_and_cached);
DEFINE_END
