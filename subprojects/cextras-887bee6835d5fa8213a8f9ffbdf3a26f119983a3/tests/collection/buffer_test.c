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
 * @file         buffer.c
 */

#include <assert.h>
#include <cextras/collection.h>
#include <string.h>
#include <testlib.h>

static void
init_buffer(void) {
	int rv;
	struct CxBuffer buffer = {0};

	rv = cx_buffer_init(&buffer);
	assert(rv == 0);
	rv = cx_buffer_cleanup(&buffer);
	assert(rv == 0);
}

static void
append_to_buffer(void) {
	int rv;
	struct CxBuffer buffer = {0};

	rv = cx_buffer_init(&buffer);
	assert(rv == 0);

	const uint8_t hello_world[] = "Hello World";
	rv = cx_buffer_append(&buffer, hello_world, sizeof(hello_world));
	assert(rv == 0);
	rv = cx_buffer_append(&buffer, hello_world, sizeof(hello_world));
	assert(rv == 0);

	const uint8_t *data = cx_buffer_data(&buffer);
	rv = memcmp(data, "Hello World\0Hello World\0", sizeof(hello_world) * 2);
	assert(rv == 0);

	rv = cx_buffer_cleanup(&buffer);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(init_buffer)
TEST(append_to_buffer)
END_TESTS
