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
 * @file        : error
 * @created     : Saturday Sep 11, 2021 12:26:48 CEST
 */

#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKOWN_ERROR_FORMAT "Unknown error %i"

static __thread char
		err_str[sizeof(UNKOWN_ERROR_FORMAT "18446744073709551615")] = {0};

void
hsqs_perror(int error_code, const char *msg) {
	if (msg) {
		fputs(msg, stderr);
		fputs(": ", stderr);
	}
	fputs(hsqs_error_str(error_code), stderr);
	fputc('\n', stderr);
}

const char *
hsqs_error_str(int error_code) {
	error_code = abs(error_code);

	if (error_code < HSQS_ERROR_SECTION) {
		return strerror(error_code);
	}
	switch ((enum HsqsError)error_code) {
	case HSQS_ERROR_SUPERBLOCK_TOO_SMALL:
		return "Superblock too small";
	case HSQS_ERROR_WRONG_MAGIG:
		return "Wrong magig";
	case HSQS_ERROR_BLOCKSIZE_MISSMATCH:
		return "Blocksize missmatch";
	case HSQS_ERROR_SIZE_MISSMATCH:
		return "Size missmatch";
	case HSQS_ERROR_CHECKFLAG_SET:
		return "Checkflag set";
	case HSQS_ERROR_METABLOCK_UNSUPPORTED_COMPRESSION:
		return "Metablock unsupported compression";
	case HSQS_ERROR_METABLOCK_INFO_IS_COMPRESSED:
		return "Metablock info is compressed";
	case HSQS_ERROR_METABLOCK_INIT:
		return "Metablock init";
	case HSQS_ERROR_COMPRESSION_INIT:
		return "Compression init";
	case HSQS_ERROR_COMPRESSION_DECOMPRESS:
		return "Compression decompress";
	case HSQS_ERROR_UNKOWN_INODE_TYPE:
		return "Unkown inode type";
	case HSQS_ERROR_COMPRESSION_STREAM_INIT:
		return "Compression stream init";
	case HSQS_ERROR_COMPRESSION_STREAM_CLEANUP:
		return "Compression stream cleanup";
	case HSQS_ERROR_STREAM_NOT_ENOUGH_BYTES:
		return "Stream not enough bytes";
	case HSQS_ERROR_GZIP_HEADER_TOO_SMALL:
		return "Gzip header too small";
	case HSQS_ERROR_GZIP_HEADER_READ:
		return "Gzip header read";
	case HSQS_ERROR_GZIP_READ_AFTER_END:
		return "Gzip read after end";
	case HSQS_ERROR_NOT_A_DIRECTORY:
		return "Not a directory";
	case HSQS_ERROR_NOT_A_FILE:
		return "Not a file";
	case HSQS_ERROR_MALLOC_FAILED:
		return "Malloc Failed";
	case HSQS_ERROR_DIRECTORY_INIT:
		return "Directory Init";
	case HSQS_ERROR_INODE_INIT:
		return "Inode init";
	case HSQS_ERROR_INTEGER_OVERFLOW:
		return "Integer overflow";
	case HSQS_ERROR_NO_SUCH_FILE:
		return "No such file or directory";
	case HSQS_ERROR_METABLOCK_ZERO_SIZE:
		return "Metablock with size zero";
	case HSQS_ERROR_SEEK_OUT_OF_RANGE:
		return "Seek out of range";
	case HSQS_ERROR_SEEK_IN_FRAGMENT:
		return "Seek in fragment";
	case HSQS_ERROR_NO_FRAGMENT:
		return "No fragment";
	case HSQS_ERROR_NO_DATABLOCKS:
		return "No datablocks";
	case HSQS_ERROR_HASHMAP_INTERNAL_ERROR:
		return "Hashmap internal error";
	case HSQS_ERROR_NO_EXTENDED_DIRECTORY:
		return "No extended directory";
	case HSQS_ERROR_NO_EXPORT_TABLE:
		return "No export table";
	case HSQS_ERROR_TODO:
		return "Todo";
	}
	snprintf(err_str, sizeof(err_str), UNKOWN_ERROR_FORMAT, abs(error_code));
	return err_str;
}
