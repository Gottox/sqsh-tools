/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         xattr_table.h
 */

#include "../context/metablock_context.h"
#include "../context/metablock_stream_context.h"
#include "../utils.h"
#include "table.h"
#include <stdint.h>

#ifndef XATTR_TABLE_H

#define XATTR_TABLE_H

struct SqshSuperblockContext;
struct SqshXattrKey;
struct SqshXattrValue;
struct SqshInodeContext;

enum SqshXattrType {
	HSQS_XATTR_USER = 0,
	HSQS_XATTR_TRUSTED = 1,
	HSQS_XATTR_SECURITY = 2,
};

struct SqshXattrTable {
	struct Sqsh *sqsh;
	struct SqshMapping header;
	struct SqshTable table;
};

HSQS_NO_UNUSED int
sqsh_xattr_table_init(struct SqshXattrTable *context, struct Sqsh *sqsh);

uint64_t sqsh_xattr_table_start(struct SqshXattrTable *table);

int sqsh_xattr_table_cleanup(struct SqshXattrTable *context);

#endif /* end of include guard XATTR_TABLE_H */
