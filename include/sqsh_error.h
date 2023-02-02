/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
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
 * @file         sqsh_error.h
 */

#ifndef SQSH_ERROR_H
#define SQSH_ERROR_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
// error.c
#define SQSH_ERROR_SECTION (1 << 8)
enum SqshError {
	SQSH_SUCCESS = 0,
	// Avoid collisions with errno
	SQSH_ERROR_SUPERBLOCK_TOO_SMALL = SQSH_ERROR_SECTION + 1,
	SQSH_ERROR_WRONG_MAGIC,
	SQSH_ERROR_BLOCKSIZE_MISSMATCH,
	SQSH_ERROR_SIZE_MISSMATCH,
	SQSH_ERROR_CHECKFLAG_SET,
	SQSH_ERROR_METABLOCK_UNSUPPORTED_COMPRESSION,
	SQSH_ERROR_METABLOCK_INFO_IS_COMPRESSED,
	SQSH_ERROR_METABLOCK_ZERO_SIZE,
	SQSH_ERROR_METABLOCK_INIT,
	SQSH_ERROR_COMPRESSION_INIT,
	SQSH_ERROR_COMPRESSION_UNSUPPORTED,
	SQSH_ERROR_COMPRESSION_DECOMPRESS,
	SQSH_ERROR_UNKOWN_INODE_TYPE,
	SQSH_ERROR_COMPRESSION_STREAM_INIT,
	SQSH_ERROR_COMPRESSION_STREAM_CLEANUP,
	SQSH_ERROR_STREAM_NOT_ENOUGH_BYTES,
	SQSH_ERROR_GZIP_HEADER_TOO_SMALL,
	SQSH_ERROR_GZIP_HEADER_READ,
	SQSH_ERROR_GZIP_READ_AFTER_END,
	SQSH_ERROR_NOT_A_DIRECTORY,
	SQSH_ERROR_NOT_A_FILE,
	SQSH_ERROR_MALLOC_FAILED,
	SQSH_ERROR_DIRECTORY_INIT,
	SQSH_ERROR_INODE_INIT,
	SQSH_ERROR_INTEGER_OVERFLOW,
	SQSH_ERROR_NO_SUCH_FILE,
	SQSH_ERROR_NO_FRAGMENT,
	SQSH_ERROR_NO_FRAGMENT_TABLE,
	SQSH_ERROR_NO_DATABLOCKS,
	SQSH_ERROR_SEEK_OUT_OF_RANGE,
	SQSH_ERROR_SEEK_IN_FRAGMENT,
	SQSH_ERROR_HASHMAP_INTERNAL_ERROR,
	SQSH_ERROR_NO_EXTENDED_DIRECTORY,
	SQSH_ERROR_NO_EXPORT_TABLE,
	SQSH_ERROR_NO_XATTR_TABLE,
	SQSH_ERROR_NO_COMPRESSION_OPTIONS,
	SQSH_ERROR_METABLOCK_TOO_BIG,
	SQSH_ERROR_MAPPER_INIT,
	SQSH_ERROR_MAPPER_MAP,
	SQSH_ERROR_INDEX_OUT_OF_BOUNDS,
	SQSH_ERROR_TODO,
};

void sqsh_perror(int error_code, const char *msg);

SQSH_NO_UNUSED const char *sqsh_error_str(int errorcode);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_ERROR_H */
