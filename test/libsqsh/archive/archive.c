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
 * @file         file.c
 */

#include "../common.h"
#include <testlib.h>

#include <sqsh_archive_private.h>
#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_file_private.h>

// The SqshConfig struct as it was in version 1.0. This is used to perform ABI
// compatibility checks.

struct SqshConfigV1_0 {
	uint64_t archive_offset;
	uint64_t source_size;
	const struct SqshMemoryMapperImpl *source_mapper;
	int mapper_block_size;
	int mapper_lru_size;
	int compression_lru_size;
	size_t max_symlink_depth;
	char _reserved[128];
};

static void
config__config_compat_check_v1_0(void) {
	ASSERT_EQ(
			offsetof(struct SqshConfig, archive_offset),
			offsetof(struct SqshConfigV1_0, archive_offset));
	ASSERT_EQ(
			offsetof(struct SqshConfig, source_size),
			offsetof(struct SqshConfigV1_0, source_size));
	ASSERT_EQ(
			offsetof(struct SqshConfig, source_mapper),
			offsetof(struct SqshConfigV1_0, source_mapper));
	ASSERT_EQ(
			offsetof(struct SqshConfig, mapper_block_size),
			offsetof(struct SqshConfigV1_0, mapper_block_size));
	ASSERT_EQ(
			offsetof(struct SqshConfig, mapper_lru_size),
			offsetof(struct SqshConfigV1_0, mapper_lru_size));
	ASSERT_EQ(
			offsetof(struct SqshConfig, compression_lru_size),
			offsetof(struct SqshConfigV1_0, compression_lru_size));
	ASSERT_EQ(
			offsetof(struct SqshConfig, max_symlink_depth),
			offsetof(struct SqshConfigV1_0, max_symlink_depth));

	ASSERT_LT(
			offsetof(struct SqshConfig, _reserved),
			sizeof(struct SqshConfigV1_0));
}

DECLARE_TESTS
TEST(config__config_compat_check_v1_0)
END_TESTS
