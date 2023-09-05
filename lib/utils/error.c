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

#ifdef _GNU_SOURCE
// We do _not_ want the gnu specific strerror_r, which returns a string, and
// sometimes doesn't use the passed buffer
#	undef _GNU_SOURCE
#endif
#define _DEFAULT_SOURCE
#include "../../include/sqsh_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKNOWN_ERROR_FORMAT "Unknown error %i"

static __thread char err_str[512] = {0};

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
		strerror_r(error_code, err_str, sizeof(err_str));
		return err_str;
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
	case SQSH_ERROR_BLOCKSIZE_MISMATCH:
		return "Blocksize mismatch";
	case SQSH_ERROR_SIZE_MISMATCH:
		return "Size mismatch";
	case SQSH_ERROR_COMPRESSION_INIT:
		return "Compression init";
	case SQSH_ERROR_COMPRESSION_DECOMPRESS:
		return "Compression decompress";
	case SQSH_ERROR_UNKNOWN_FILE_TYPE:
		return "Unknown file type";
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
	case SQSH_ERROR_NO_SUCH_XATTR:
		return "No such xattr";
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
		return "Compression unknown";
	case SQSH_ERROR_CURL_INVALID_RANGE_HEADER:
		return "Invalid range header";
	case SQSL_ERROR_ELEMENT_NOT_FOUND:
		return "Element not found";
	case SQSH_ERROR_INVALID_ARGUMENT:
		return "Invalid argument";
	case SQSH_ERROR_WALKER_CANNOT_GO_UP:
		return "Walker cannot go up";
	case SQSH_ERROR_WALKER_CANNOT_GO_DOWN:
		return "Walker cannot go down";
	case SQSH_ERROR_CORRUPTED_INODE:
		return "Corrupted inode";
	case SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY:
		return "Corrupted directory entry";
	case SQSH_ERROR_INTERNAL:
		return "Internal error";
	case SQSH_ERROR_INODE_MAP_IS_INCONSISTENT:
		return "Inode map is inconsistent";
	case SQSH_ERROR_XATTR_SIZE_MISMATCH:
		return "Xattr size mismatch";
	case SQSH_ERROR_UNSUPPORTED_VERSION:
		return "Unsupported version";
	case SQSH_ERROR_TOO_MANY_SYMLINKS_FOLLOWED:
		return "Too many symlinks followed";
	case SQSH_ERROR_CORRUPTED_DIRECTORY_HEADER:
		return "Corrupted directory header";
	case SQSH_ERROR_COMPRESSION_FINISHED:
		return "Compression already finished";
	case SQSH_ERROR_NO_SUCH_ELEMENT:
		return "No such element";
	}
	snprintf(err_str, sizeof(err_str), UNKNOWN_ERROR_FORMAT, error_code);
	return err_str;
}
