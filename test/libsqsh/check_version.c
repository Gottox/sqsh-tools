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
 * @file         nasty.c
 */

#include <sqsh_utils.h>
#include <utest.h>

UTEST(version, version_defines_are_correct) {
	char *version = getenv("VERSION");

	ASSERT_NE(NULL, version);
	ASSERT_STREQ(SQSH_VERSION, version);
	ASSERT_STREQ(SQSH_VERSION, sqsh_version());

	int major, minor, patch;
	ASSERT_EQ(3, sscanf(version, "%d.%d.%d", &major, &minor, &patch));

	ASSERT_EQ(SQSH_VERSION_MAJOR, major);
	ASSERT_EQ(SQSH_VERSION_MAJOR, sqsh_version_major());
	ASSERT_EQ(SQSH_VERSION_MINOR, minor);
	ASSERT_EQ(SQSH_VERSION_MINOR, sqsh_version_minor());
	ASSERT_EQ(SQSH_VERSION_PATCH, patch);
	ASSERT_EQ(SQSH_VERSION_PATCH, sqsh_version_patch());
}

UTEST_MAIN()
