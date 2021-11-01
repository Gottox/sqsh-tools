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
 * @file        : squash
 * @created     : Friday Apr 30, 2021 10:58:14 CEST
 */

#include "context/superblock_context.h"
#include "error.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>

#ifndef HSQS_H

#define HSQS_H

enum HsqsDtor {
	HSQS_DTOR_NONE,
	HSQS_DTOR_FREE,
	HSQS_DTOR_MUNMAP,
};

struct Hsqs {
	uint32_t error;
	struct HsqsSuperblockContext superblock;
	int size;
	uint8_t *buffer;
	enum HsqsDtor dtor;
};

HSQS_NO_UNUSED int hsqs_init(
		struct Hsqs *squash, uint8_t *buffer, const size_t size,
		const enum HsqsDtor dtor);

HSQS_NO_UNUSED int hsqs_open(struct Hsqs *squash, const char *path);

int hsqs_cleanup(struct Hsqs *squash);

#endif /* end of include guard HSQS_H */
