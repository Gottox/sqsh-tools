/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         cpp-test.cpp
 */

#ifndef __cplusplus
#	error "This file is C++ only"
#endif

#include "common.h"
#include <sqsh_archive.h>
#include <sqsh_directory.h>
#include <sqsh_error.h>
#include <sqsh_mapper.h>
#include <testlib.h>

static void
sqsh_empty(void) {
	int rv;
	struct SqshArchive *archive = NULL;
	struct SqshConfig config = {};
	config.source_mapper = sqsh_mapper_impl_static;
	config.mapper_block_size = 1;
	config.source_size = 0;
	archive = sqsh_archive_open(NULL, &config, &rv);
	ASSERT_EQ(-SQSH_ERROR_SUPERBLOCK_TOO_SMALL, rv);
	// BUG: Not using ASSERT_EQ here, because of this issue:
	// https://github.com/sheredom/utest.h/issues/151
	ASSERT_TRUE(archive == NULL);
}

DECLARE_TESTS
TEST(sqsh_empty)
END_TESTS
