/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2018, Enno Boland
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
 * @file         lru_hashmap.c
 */

#include "../common.h"
#include "../test.h"

#include "../../src/buffer/buffer.h"
#include "../../src/buffer/cow.h"
#include "../../src/mapper/mapper.h"

static const int compression_id = HSQS_COMPRESSION_GZIP;

int
buffer_dtor(void *data) {
	struct HsqsMapping *m = data;
	return hsqs_mapping_unmap(m);
}

static void
init_cow() {
	int rv = 0;
	struct HsqsCow cow = {0};

	rv = hsqs_cow_init(&cow, compression_id, 1024);
	assert(rv == 0);

	hsqs_cow_cleanup(&cow);
}

static void
add_to_cow_with_append() {
	int rv = 0;

	struct HsqsRefCount *ref = NULL;
	struct HsqsCow cow = {0};
	struct HsqsMapper mapper = {0};
	struct HsqsMapping *mapping = NULL;
	static const uint8_t data[] = "0123456789";
	static const uint8_t datadata[] = "01234567890123456789";

	rv = hsqs_mapper_init_static(&mapper, data, sizeof(data) - 1);

	rv = hsqs_ref_count_new(&ref, sizeof(struct HsqsMapping), buffer_dtor);
	assert(rv == 0);
	mapping = hsqs_ref_count_retain(ref);

	rv = hsqs_mapper_map(mapping, &mapper, 0, sizeof(data) - 1);
	assert(rv == 0);

	rv = hsqs_cow_init(&cow, compression_id, 1024);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_EMPTY);

	rv = hsqs_cow_append_block(&cow, ref, 0, sizeof(data) - 1, false);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_PASS_THROUGH);
	assert(hsqs_cow_data(&cow) == data);

	rv = hsqs_cow_append_block(&cow, ref, 0, sizeof(data) - 1, false);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_BUFFERED);
	assert(memcmp(hsqs_cow_data(&cow), datadata, sizeof(datadata) - 1) == 0);

	rv = hsqs_ref_count_release(ref);
	assert(rv >= 0);

	rv = hsqs_cow_cleanup(&cow);
	assert(rv >= 0);
}

static void
add_to_cow_with_offset() {
	int rv = 0;

	struct HsqsRefCount *ref = NULL;
	struct HsqsCow cow = {0};
	struct HsqsMapper mapper = {0};
	struct HsqsMapping *mapping = NULL;
	static const uint8_t data[] = "pre_0123456789_suf";
	static const uint8_t datadataoffset[] = "01234567890123456789";
	static int offset = 4;
	static int size = 10;

	rv = hsqs_mapper_init_static(&mapper, data, sizeof(data) - 1);

	rv = hsqs_ref_count_new(&ref, sizeof(struct HsqsMapping), buffer_dtor);
	assert(rv == 0);
	mapping = hsqs_ref_count_retain(ref);

	rv = hsqs_mapper_map(mapping, &mapper, 0, sizeof(data) - 1);
	assert(rv == 0);

	rv = hsqs_cow_init(&cow, compression_id, 1024);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_EMPTY);

	rv = hsqs_cow_append_block(&cow, ref, offset, size, false);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_PASS_THROUGH);
	assert(hsqs_cow_data(&cow) == &data[offset]);

	rv = hsqs_cow_append_block(&cow, ref, offset, size, false);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_BUFFERED);
	assert(memcmp(hsqs_cow_data(&cow), datadataoffset,
				  sizeof(datadataoffset) - 1) == 0);

	rv = hsqs_ref_count_release(ref);
	assert(rv >= 0);

	rv = hsqs_cow_cleanup(&cow);
	assert(rv >= 0);
}

static void
add_to_cow_with_compression() {
	int rv = 0;

	struct HsqsRefCount *ref = NULL;
	struct HsqsCow cow = {0};
	struct HsqsMapper mapper = {0};
	struct HsqsMapping *mapping = NULL;
	static const uint8_t data_compressed[] = {
			0x78, 0x5e, 0x33, 0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33,
			0xb7, 0xb0, 0x04, 0x00, 0x0a, 0xff, 0x02, 0x0e, 0};
	static const uint8_t data_uncompressed[] = "0123456789";

	rv = hsqs_mapper_init_static(
			&mapper, data_compressed, sizeof(data_compressed) - 1);

	rv = hsqs_ref_count_new(&ref, sizeof(struct HsqsMapping), buffer_dtor);
	assert(rv == 0);
	mapping = hsqs_ref_count_retain(ref);

	rv = hsqs_mapper_map(mapping, &mapper, 0, sizeof(data_compressed) - 1);
	assert(rv == 0);

	rv = hsqs_cow_init(&cow, compression_id, 1024);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_EMPTY);

	rv = hsqs_cow_append_block(&cow, ref, 0, sizeof(data_compressed) - 1, true);
	assert(rv == 0);

	assert(cow.state == HSQS_COW_BUFFERED);
	assert(memcmp(hsqs_cow_data(&cow), data_uncompressed,
				  sizeof(data_uncompressed) - 1) == 0);

	rv = hsqs_ref_count_release(ref);
	assert(rv >= 0);

	rv = hsqs_cow_cleanup(&cow);
	assert(rv >= 0);
}

DEFINE
TEST(init_cow);
TEST(add_to_cow_with_append);
TEST(add_to_cow_with_offset);
TEST(add_to_cow_with_compression);
DEFINE_END
