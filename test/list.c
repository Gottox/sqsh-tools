/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @file        : list
 * @created     : Monday Oct 11, 2021 21:43:12 CEST
 */

#include "../src/context/directory_context.h"
#include "../src/context/inode_context.h"
#include "../src/context/superblock_context.h"
#include "../src/data/superblock.h"
#include "../src/error.h"
#include "common.h"
#include "squash_image.h"
#include "test.h"

static void
squash_ls() {
	int rv;
	char *name;
	struct SquashSuperblockContext superblock = {0};
	struct SquashInodeContext inode = {0};
	struct SquashDirectoryContext dir = {0};
	struct SquashDirectoryIterator iter = {0};
	rv = squash_superblock_init(
			&superblock, squash_image, sizeof(squash_image));
	assert(rv == 0);

	rv = squash_inode_load(&inode, &superblock,
			squash_data_superblock_root_inode_ref(superblock.superblock));
	assert(rv == 0);

	rv = squash_directory_init(&dir, &superblock, &inode);
	assert(rv == 0);

	rv = squash_directory_iterator_init(&iter, &dir);
	assert(rv == 0);

	rv = squash_directory_iterator_next(&iter);
	assert(rv > 0);
	rv = squash_directory_iterator_name_dup(&iter, &name);
	assert(rv == 1);
	assert(strcmp("a", name) == 0);
	free(name);

	rv = squash_directory_iterator_next(&iter);
	assert(rv >= 0);
	rv = squash_directory_iterator_name_dup(&iter, &name);
	assert(rv == 1);
	assert(strcmp("b", name) == 0);
	free(name);

	rv = squash_directory_iterator_next(&iter);
	// End of file list
	assert(rv == 0);

	rv = squash_directory_iterator_cleanup(&iter);
	assert(rv == 0);

	rv = squash_directory_cleanup(&dir);
	assert(rv == 0);

	rv = squash_inode_cleanup(&inode);
	assert(rv == 0);

	rv = squash_superblock_cleanup(&superblock);
	assert(rv == 0);
}

DEFINE
TEST(squash_ls);
DEFINE_END
