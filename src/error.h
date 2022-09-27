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
 * @file         error.h
 */

#include "utils.h"

#ifndef HSQS_ERROR_H

#define HSQS_ERROR_H

#define HSQS_ERROR_SECTION (1 << 8)
enum SqshError {
	// Avoid collisions with errno
	HSQS_ERROR_SUPERBLOCK_TOO_SMALL = HSQS_ERROR_SECTION + 1,
	HSQS_ERROR_WRONG_MAGIC,
	HSQS_ERROR_BLOCKSIZE_MISSMATCH,
	HSQS_ERROR_SIZE_MISSMATCH,
	HSQS_ERROR_CHECKFLAG_SET,
	HSQS_ERROR_METABLOCK_UNSUPPORTED_COMPRESSION,
	HSQS_ERROR_METABLOCK_INFO_IS_COMPRESSED,
	HSQS_ERROR_METABLOCK_ZERO_SIZE,
	HSQS_ERROR_METABLOCK_INIT,
	HSQS_ERROR_COMPRESSION_INIT,
	HSQS_ERROR_COMPRESSION_UNSUPPORTED,
	HSQS_ERROR_COMPRESSION_DECOMPRESS,
	HSQS_ERROR_UNKOWN_INODE_TYPE,
	HSQS_ERROR_COMPRESSION_STREAM_INIT,
	HSQS_ERROR_COMPRESSION_STREAM_CLEANUP,
	HSQS_ERROR_STREAM_NOT_ENOUGH_BYTES,
	HSQS_ERROR_GZIP_HEADER_TOO_SMALL,
	HSQS_ERROR_GZIP_HEADER_READ,
	HSQS_ERROR_GZIP_READ_AFTER_END,
	HSQS_ERROR_NOT_A_DIRECTORY,
	HSQS_ERROR_NOT_A_FILE,
	HSQS_ERROR_MALLOC_FAILED,
	HSQS_ERROR_DIRECTORY_INIT,
	HSQS_ERROR_INODE_INIT,
	HSQS_ERROR_INTEGER_OVERFLOW,
	HSQS_ERROR_NO_SUCH_FILE,
	HSQS_ERROR_NO_FRAGMENT,
	HSQS_ERROR_NO_FRAGMENT_TABLE,
	HSQS_ERROR_NO_DATABLOCKS,
	HSQS_ERROR_SEEK_OUT_OF_RANGE,
	HSQS_ERROR_SEEK_IN_FRAGMENT,
	HSQS_ERROR_HASHMAP_INTERNAL_ERROR,
	HSQS_ERROR_NO_EXTENDED_DIRECTORY,
	HSQS_ERROR_NO_EXPORT_TABLE,
	HSQS_ERROR_NO_XATTR_TABLE,
	HSQS_ERROR_NO_COMPRESSION_OPTIONS,
	HSQS_ERROR_METABLOCK_TOO_BIG,
	HSQS_ERROR_MAPPER_INIT,
	HSQS_ERROR_MAPPER_MAP,
	HSQS_ERROR_TODO,
};

void sqsh_perror(int error_code, const char *msg);

HSQS_NO_UNUSED const char *sqsh_error_str(int errorcode);

#endif /* end of include guard HSQS_ERROR_H */
