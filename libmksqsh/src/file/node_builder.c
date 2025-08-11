/******************************************************************************
 *                                                                            *
 * Copyright (c) 2024, Enno Boland <g@s01.de>                                 *
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
 * @file         simple_archive.c
 *
 * This file implements a very small SquashFS writer that is only capable of
 * creating archives that contain a handful of files in the root directory.
 * The implementation is deliberately tiny and only implements what is needed
 * for the unit tests in this repository.  It should not be used as a full
 * featured replacement for mksquashfs.
 */

#define _DEFAULT_SOURCE

#include <mksqsh_archive_private.h>
#include <mksqsh_metablock.h>

#include <sqsh_common_private.h>
#include <sqsh_data_private.h>
#include <sqsh_error.h>

#include <cextras/memory.h>
#include <endian.h>
#include <mksqsh_file_private.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct MksqshNode *
mksqsh__node_retain(struct MksqshNode *node) {
	if (node != NULL) {
		cx_rc_retain(&node->rc);
	}
	return node;
}

int
mksqsh__node_init(struct MksqshNode *node, struct MksqshFile *file) {
	memset(node, 0, sizeof(*node));
	cx_rc_init(&node->rc);
	node->file = mksqsh_file_retain(file);
	return 0;
}
