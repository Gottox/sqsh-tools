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
 * @file         sqsh.h
 */

#include "sqsh_common.h"
#include "sqsh_compression.h"
#include "sqsh_context.h"
#include "sqsh_error.h"
#include "sqsh_mapper.h"
#include "sqsh_table.h"

#include <sqsh.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SQSH_PRIVATE_H

#define SQSH_PRIVATE_H

struct SqshTrailingContext;

struct Sqsh {
	uint32_t error;
	struct SqshMapper mapper;
	struct SqshCompression data_compression;
	struct SqshCompression metablock_compression;
	struct SqshMapper table_mapper;
	struct SqshMapping table_map;
	struct SqshSuperblockContext superblock;
	struct SqshTable id_table;
	struct SqshTable export_table;
	struct SqshXattrTable xattr_table;
	struct SqshFragmentTable fragment_table;
	struct SqshCompressionOptionsContext compression_options;
	uint8_t initialized;
	struct SqshConfig config;
};

#endif /* end of include guard SQSH_PRIVATE_H */