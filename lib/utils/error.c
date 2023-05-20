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
 * @file         error.c
 */

#include "../../include/sqsh_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKOWN_ERROR_FORMAT "Unknown error %i"

static __thread char
		err_str[sizeof(UNKOWN_ERROR_FORMAT "18446744073709551615")] = {0};

void
sqsh_perror(int error_code, const char *msg) {
	if (msg) {
		fputs(msg, stderr);
		fputs(": ", stderr);
	}
	fputs(sqsh_error_str(error_code), stderr);
	fputc('\n', stderr);
}

const char *
sqsh_error_str(int error_code) {
	error_code = abs(error_code);

	if (error_code < SQSH_ERROR_SECTION_START) {
		return strerror(error_code);
	}
	switch ((enum SqshError)error_code) {
	case SQSH_ERROR_SECTION_START:
		break;
	case SQSH_SUCCESS:
		return "Success";
	case SQSH_ERROR_NO_COMPRESSION_OPTIONS:
		return "No compression options";
	case SQSH_ERROR_SUPERBLOCK_TOO_SMALL:
		return "Superblock too small";
	case SQSH_ERROR_WRONG_MAGIC:
		return "Wrong magic";
	case SQSH_ERROR_BLOCKSIZE_MISSMATCH:
		return "Blocksize missmatch";
	case SQSH_ERROR_SIZE_MISSMATCH:
		return "Size missmatch";
	case SQSH_ERROR_COMPRESSION_INIT:
		return "Compression init";
	case SQSH_ERROR_COMPRESSION_DECOMPRESS:
		return "Compression decompress";
	case SQSH_ERROR_UNKOWN_INODE_TYPE:
		return "Unkown inode type";
	case SQSH_ERROR_NOT_A_DIRECTORY:
		return "Not a directory";
	case SQSH_ERROR_NOT_A_FILE:
		return "Not a file";
	case SQSH_ERROR_MALLOC_FAILED:
		return "Malloc Failed";
	case SQSH_ERROR_MUTEX_INIT_FAILED:
		return "Mutex init failed";
	case SQSH_ERROR_MUTEX_LOCK_FAILED:
		return "Mutex lock failed";
	case SQSH_ERROR_MUTEX_DESTROY_FAILED:
		return "Mutex destroy failed";
	case SQSH_ERROR_OUT_OF_BOUNDS:
		return "Out of bounds";
	case SQSH_ERROR_INTEGER_OVERFLOW:
		return "Integer overflow";
	case SQSH_ERROR_NO_SUCH_FILE:
		return "No such file or directory";
	case SQSH_ERROR_NO_EXTENDED_DIRECTORY:
		return "No extended directory";
	case SQSH_ERROR_NO_FRAGMENT_TABLE:
		return "No fragment table";
	case SQSH_ERROR_NO_EXPORT_TABLE:
		return "No export table";
	case SQSH_ERROR_NO_XATTR_TABLE:
		return "No xattr table";
	case SQSH_ERROR_MAPPER_INIT:
		return "Mapper init error";
	case SQSH_ERROR_MAPPER_MAP:
		return "Mapper mapping error";
	case SQSH_ERROR_COMPRESSION_UNSUPPORTED:
		return "Compression unkown";
	case SQSH_ERROR_CURL_INVALID_RANGE_HEADER:
		return "Invalid range header";
	case SQSL_ERROR_ELEMENT_NOT_FOUND:
		return "Element not found";
	case SQSH_ERROR_INVALID_ARGUMENT:
		return "Invalid argument";
	case SQSH_ERROR_TODO:
		return "Todo";
	}
	snprintf(err_str, sizeof(err_str), UNKOWN_ERROR_FORMAT, abs(error_code));
	return err_str;
}
